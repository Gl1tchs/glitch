#include <doctest/doctest.h>

#include "glitch/asset/asset.h"
#include "glitch/asset/asset_system.h"
#include "glitch/platform/os.h"

using namespace gl;

struct MockCreatableAsset {
	GL_REFLECT_ASSET("MockCreatableAsset");

	int value;
	inline static bool s_force_create_failure = false;

	MockCreatableAsset(int v) : value(v) {}

	static std::shared_ptr<MockCreatableAsset> create(int p_value) {
		if (s_force_create_failure) {
			return nullptr;
		}
		return std::make_shared<MockCreatableAsset>(p_value);
	}
};

struct MockLoadableAsset {
	GL_REFLECT_ASSET("MockLoadableAsset");

	int value;
	fs::path loaded_from;
	inline static bool s_force_load_failure = false;

	MockLoadableAsset(int v, const fs::path& p) : value(v), loaded_from(p) {}

	static std::shared_ptr<MockLoadableAsset> load(const fs::path& p_path, int p_value) {
		if (s_force_load_failure) {
			return nullptr; // simulate a parsing error
		}

		// Simulate a successful load only for a specific path
		if (p_path == fs::path("/home/glitch/test_asset.dat")) {
			return std::make_shared<MockLoadableAsset>(p_value, p_path);
		}

		return nullptr; // simulate file not found at path
	}
};

struct AnotherMockAsset {
	GL_REFLECT_ASSET("AnotherMockAsset");

	static std::shared_ptr<AnotherMockAsset> create() {
		return std::make_shared<AnotherMockAsset>();
	}
};

struct MockSerializedAsset {
	GL_REFLECT_ASSET("MockSerializedAsset");

	std::string data;
	fs::path loaded_path;

	MockSerializedAsset(std::string p_data, fs::path p_path) :
			data(std::move(p_data)), loaded_path(std::move(p_path)) {}

	// The deserialize method in AssetRegistry expects this signature
	static std::shared_ptr<MockSerializedAsset> load(const fs::path& p_path) {
		// Simulate file reading based on path
		if (p_path == "/home/glitch/save_data.json") {
			return std::make_shared<MockSerializedAsset>("Restored Data", p_path);
		}
		return nullptr;
	}
};

// --- Concept Sanity Checks ---
static_assert(IsCreatableAsset<MockCreatableAsset, int>);
static_assert(IsLoadableAsset<MockLoadableAsset, int>);
static_assert(IsCreatableAsset<AnotherMockAsset>);
static_assert(IsLoadableAsset<MockSerializedAsset>);

// --- Test Cases ---

TEST_CASE("Test asset path conversion") {
	CHECK(os::setenv("GL_WORKING_DIR", "/home/glitch"));

	auto path_res = AssetSystem::get_absolute_path("res://script.lua");
	REQUIRE(path_res.has_value());
	CHECK(path_res.get_value() == "/home/glitch/script.lua");

	CHECK(AssetSystem::get_absolute_path("").get_error() == PathProcessError::EMPTY_PATH);
	CHECK(AssetSystem::get_absolute_path("ref://script.lua").get_error() ==
			PathProcessError::INVALID_IDENTIFIER);

	CHECK(os::setenv("GL_WORKING_DIR", ""));

	CHECK(AssetSystem::get_absolute_path("res://script.lua").get_error() ==
			PathProcessError::UNDEFINED_WORKING_DIR);

	// Cleanup env
	CHECK(os::setenv("GL_WORKING_DIR", "/home/glitch"));
}

TEST_CASE("AssetSystem Lifecycle (Load, Create, Get, Free, GC, Shutdown)") {
	CHECK(os::setenv("GL_WORKING_DIR", "/home/glitch"));

	// Reset mock asset static flags
	MockLoadableAsset::s_force_load_failure = false;
	MockCreatableAsset::s_force_create_failure = false;

	AssetHandle h_create_valid;
	AssetHandle h_load_valid;

	SUBCASE("Create and Get") {
		std::optional<AssetHandle> handle_opt = AssetSystem::create<MockCreatableAsset>(42);
		REQUIRE(handle_opt.has_value());
		h_create_valid = *handle_opt;
		CHECK(h_create_valid.is_valid());

		auto asset_create = AssetSystem::get<MockCreatableAsset>(h_create_valid);
		REQUIRE(asset_create != nullptr);
		CHECK(asset_create->value == 42);
	}

	SUBCASE("Create Failure") {
		MockCreatableAsset::s_force_create_failure = true;
		auto handle_opt = AssetSystem::create<MockCreatableAsset>(99);
		CHECK(!handle_opt.has_value());
		MockCreatableAsset::s_force_create_failure = false; // reset flag
	}

	SUBCASE("Load and Get") {
		auto handle_res = AssetSystem::load<MockLoadableAsset>("res://test_asset.dat", 123);
		REQUIRE(handle_res.has_value());
		h_load_valid = *handle_res;
		CHECK(h_load_valid.is_valid());

		auto asset_load = AssetSystem::get<MockLoadableAsset>(h_load_valid);
		REQUIRE(asset_load != nullptr);
		CHECK(asset_load->value == 123);
		CHECK(asset_load->loaded_from == fs::path("/home/glitch/test_asset.dat"));
	}

	SUBCASE("Load Failure (Path Error)") {
		auto handle_res = AssetSystem::load<MockLoadableAsset>("bad://path.dat", 0);
		REQUIRE(handle_res.has_error());
		CHECK(handle_res.get_error() == AssetLoadingError::FILE_ERROR);
	}

	SUBCASE("Load Failure (Parsing Error)") {
		MockLoadableAsset::s_force_load_failure = true;
		auto handle_res = AssetSystem::load<MockLoadableAsset>("res://test_asset.dat", 0);
		REQUIRE(handle_res.has_error());
		CHECK(handle_res.get_error() == AssetLoadingError::PARSING_ERROR);
		MockLoadableAsset::s_force_load_failure = false; // reset
	}

	SUBCASE("Get Failure (Invalid Handle)") {
		AssetHandle invalid_handle; // Default, invalid UID
		auto asset_invalid = AssetSystem::get<MockCreatableAsset>(invalid_handle);
		CHECK(asset_invalid == nullptr);
	}

	SUBCASE("Get Failure (Wrong Type)") {
		auto h_opt = AssetSystem::create<MockCreatableAsset>(10);
		REQUIRE(h_opt.has_value());
		// Try to get as a different asset type
		auto asset_wrong_type = AssetSystem::get<AnotherMockAsset>(*h_opt);
		CHECK(asset_wrong_type == nullptr);
	}

	SUBCASE("Register Asset (Manual)") {
		auto manual_asset = std::make_shared<MockCreatableAsset>(777);
		AssetHandle h_manual = AssetSystem::register_asset(manual_asset);
		CHECK(h_manual.is_valid());

		auto asset_manual_get = AssetSystem::get<MockCreatableAsset>(h_manual);
		REQUIRE(asset_manual_get != nullptr);
		CHECK(asset_manual_get->value == 777);
		CHECK(asset_manual_get.get() == manual_asset.get()); // Same pointer
	}

	SUBCASE("Free Asset") {
		auto h_free_opt = AssetSystem::create<MockCreatableAsset>(1);
		REQUIRE(h_free_opt.has_value());
		AssetHandle h_free = *h_free_opt;

		auto asset_free_check = AssetSystem::get<MockCreatableAsset>(h_free);
		REQUIRE(asset_free_check != nullptr); // Check it's there

		bool freed = AssetSystem::free<MockCreatableAsset>(h_free);
		CHECK(freed == true);

		asset_free_check = AssetSystem::get<MockCreatableAsset>(h_free);
		CHECK(asset_free_check == nullptr); // Check it's gone
	}

	SUBCASE("Free Failure (Invalid Handle)") {
		AssetHandle invalid_handle;
		bool freed_invalid = AssetSystem::free<MockCreatableAsset>(invalid_handle);
		CHECK(freed_invalid == false);
	}

	SUBCASE("Garbage Collection") {
		// Create an asset and hold a reference to it
		auto h_gc_keep_opt = AssetSystem::create<MockCreatableAsset>(100);
		REQUIRE(h_gc_keep_opt.has_value());
		AssetHandle h_gc_keep = *h_gc_keep_opt;
		std::shared_ptr<MockCreatableAsset> my_ref =
				AssetSystem::get<MockCreatableAsset>(h_gc_keep);
		// use_count is at least 2 (my_ref + registry)
		REQUIRE(my_ref != nullptr);

		// Create an asset and don't hold a reference
		auto h_gc_remove_opt = AssetSystem::create<MockCreatableAsset>(200);
		REQUIRE(h_gc_remove_opt.has_value());
		AssetHandle h_gc_remove = *h_gc_remove_opt;
		// use_count in registry is 1

		// Call GC
		AssetSystem::collect_garbage();

		// Check assets
		auto asset_gc_keep = AssetSystem::get<MockCreatableAsset>(h_gc_keep);
		CHECK(asset_gc_keep != nullptr); // Should still be there
		CHECK(asset_gc_keep->value == 100);

		auto asset_gc_remove = AssetSystem::get<MockCreatableAsset>(h_gc_remove);
		CHECK(asset_gc_remove == nullptr); // Should be gone

		// Release our reference and GC again
		my_ref.reset();
		asset_gc_keep.reset();
		// use_count for h_gc_keep in registry is now 1

		AssetSystem::collect_garbage();

		asset_gc_keep = AssetSystem::get<MockCreatableAsset>(h_gc_keep);
		CHECK(asset_gc_keep == nullptr); // Now it should be gone
	}

	SUBCASE("Shutdown") {
		// Create one last asset
		auto h_shutdown_opt = AssetSystem::create<AnotherMockAsset>();
		REQUIRE(h_shutdown_opt.has_value());
		AssetHandle h_shutdown = *h_shutdown_opt;

		auto asset_shutdown_check = AssetSystem::get<AnotherMockAsset>(h_shutdown);
		REQUIRE(asset_shutdown_check != nullptr); // It's there

		// Call shutdown
		AssetSystem::clear();

		// Check if asset is gone
		asset_shutdown_check = AssetSystem::get<AnotherMockAsset>(h_shutdown);
		CHECK(asset_shutdown_check == nullptr);

		// Check if assets from other subcases are also gone
		auto asset_create_after_shutdown = AssetSystem::get<MockCreatableAsset>(h_create_valid);
		CHECK(asset_create_after_shutdown == nullptr);
	}

	CHECK(os::setenv("GL_WORKING_DIR", "")); // Reset env
}

TEST_CASE("AssetSystem Serialization") {
	AssetSystem::clear();
	os::setenv("GL_WORKING_DIR", "/home/glitch");

	SUBCASE("Serialize and Deserialize Loadable Assets") {
		AssetHandle original_handle;

		{
			// We manually register a "loaded" asset to simulate a valid state
			// because we want to test the serialization logic specifically.
			auto asset = std::make_shared<MockSerializedAsset>(
					"Original Data", "/home/glitch/save_data.json");

			// Register with a path so it is treated as a file-based asset
			original_handle = AssetSystem::register_asset(asset, "/home/glitch/save_data.json");

			REQUIRE(original_handle.is_valid());
		}

		// Serialize: Save the state to JSON
		json serialized_data;
		AssetSystem::serialize(serialized_data);

		// Verify JSON structure
		CHECK(serialized_data.contains("MockSerializedAsset"));
		CHECK(serialized_data["MockSerializedAsset"].is_array());
		CHECK(serialized_data["MockSerializedAsset"].size() == 1);
		CHECK(serialized_data["MockSerializedAsset"][0]["path"] == "/home/glitch/save_data.json");

		AssetHandle saved_handle = serialized_data["MockSerializedAsset"][0]["handle"];
		CHECK(saved_handle == original_handle);

		//  Reset: Shutdown AssetSystem to clear memory
		AssetSystem::clear();

		// Verify asset is gone
		CHECK(AssetSystem::get<MockSerializedAsset>(original_handle) == nullptr);

		AssetSystem::deserialize(serialized_data);

		auto restored_asset = AssetSystem::get<MockSerializedAsset>(original_handle);
		REQUIRE(restored_asset != nullptr);
		CHECK(restored_asset->data == "Restored Data"); // Value set by static load()
		CHECK(restored_asset->loaded_path == "/home/glitch/save_data.json");
	}

	SUBCASE("Non-Loadable Assets are NOT Serialized") {
		// MockCreatableAsset is creatable but not loadable (no static load(path) fn)
		// or simply has no path associated in registry.

		auto handle_opt = AssetSystem::create<MockCreatableAsset>(999);
		REQUIRE(handle_opt.has_value());

		json serialized_data;
		AssetSystem::serialize(serialized_data);

		// AssetRegistry::serialize checks 'if constexpr (IsLoadableAsset<T>)'
		// Even if it did, assets created via create() usually have empty paths.
		// The MockCreatableAsset shouldn't appear in the output.
		CHECK_FALSE(serialized_data.contains("MockCreatableAsset"));
	}

	SUBCASE("Memory-only Assets are NOT Serialized") {
		// Manually register a loadable asset but with a "mem://" path
		auto asset =
				std::make_shared<MockSerializedAsset>("Mem Data", "mem://MockSerializedAsset/test");
		AssetSystem::register_asset(asset, "mem://MockSerializedAsset/test");

		json serialized_data;
		AssetSystem::serialize(serialized_data);

		// Logic in AssetRegistry::serialize skips "mem://" prefix
		if (serialized_data.contains("MockSerializedAsset")) {
			CHECK(serialized_data["MockSerializedAsset"].empty());
		} else {
			CHECK(!serialized_data.contains("MockSerializedAsset"));
		}
	}

	SUBCASE("Deserialization skips missing files") {
		// create JSON manually to simulate a save file pointing to a non-existent asset
		json fake_save;
		fake_save["MockSerializedAsset"] = json::array({
				{
						{ "handle", AssetHandle() }, // Random valid UID
						{ "path", "/home/glitch/missing_file.json" },
				},
		});

		AssetSystem::deserialize(fake_save);

		// Iterate registries to see if anything was added
		auto metadata = AssetSystem::get_asset_metadatas();
		CHECK(metadata.empty());
	}

	// Cleanup
	AssetSystem::clear();
	os::setenv("GL_WORKING_DIR", "");
}