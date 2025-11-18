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

// --- Concept Sanity Checks ---
static_assert(IsCreatableAsset<MockCreatableAsset, int>);
static_assert(IsLoadableAsset<MockLoadableAsset, int>);
static_assert(IsCreatableAsset<AnotherMockAsset>);

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

	AssetSystem::init();

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
		AssetSystem::shutdown();

		// Check if asset is gone
		asset_shutdown_check = AssetSystem::get<AnotherMockAsset>(h_shutdown);
		CHECK(asset_shutdown_check == nullptr);

		// Check if assets from other subcases are also gone
		auto asset_create_after_shutdown = AssetSystem::get<MockCreatableAsset>(h_create_valid);
		CHECK(asset_create_after_shutdown == nullptr);
	}

	// Final cleanup
	AssetSystem::shutdown(); // Ensure clean state for next test case
	CHECK(os::setenv("GL_WORKING_DIR", "")); // Reset env
}
