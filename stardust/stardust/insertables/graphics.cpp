#include "graphics.h"

namespace stardust {
	const char* Graphics::GRAPHICS_FOLDER_NAME = "Graphics";

	Graphics::Graphics(const Configuration& config)
		: LunarMagicInsertable(config), 
		project_graphics_folder_path(config.project_rom.getOrThrow().parent_path() / GRAPHICS_FOLDER_NAME),
		temporary_graphics_folder_path(temporary_rom_path.parent_path() / GRAPHICS_FOLDER_NAME)
	{
		registerConfigurationDependency(config.project_rom, Policy::REINSERT);

		if (!fs::exists(project_graphics_folder_path)) {
			throw ResourceNotFoundException(fmt::format(
				"Graphics folder not found at {}",
				project_graphics_folder_path.string()
			));
		}
	}

	void Graphics::createTemporaryGraphicsFolder() const {
		if (temporary_graphics_folder_path != project_graphics_folder_path) {
			spdlog::debug(fmt::format(
				"Copying project Graphics folder {} to temporary Graphics folder {}",
				project_graphics_folder_path.string(),
				temporary_graphics_folder_path.string()
			));
			try {
				fs::copy(project_graphics_folder_path, temporary_graphics_folder_path, fs::copy_options::overwrite_existing);
			}
			catch (const fs::filesystem_error&) {
				throw InsertionException(fmt::format(
					"Failed to copy project Graphics folder {} to temporary Graphics folder {}",
					project_graphics_folder_path.string(),
					temporary_graphics_folder_path.string()
				));
			}
		}
		else {
			spdlog::debug(fmt::format(
				"Project output ROM and temporary ROM are in same folder, no need to copy Graphics folder"
			));
		}
	}

	void Graphics::deleteTemporaryGraphicsFolder() const {
		if (temporary_graphics_folder_path != project_graphics_folder_path) {
			spdlog::debug(fmt::format(
				"Deleting temporary Graphics folder {}",
				temporary_graphics_folder_path.string()
			));
			try {
				fs::remove_all(temporary_graphics_folder_path);
			}
			catch (const fs::filesystem_error&) {
				spdlog::warn(fmt::format(
					"Failed to delete temporary Graphics folder {}",
					temporary_graphics_folder_path.string()
				));
			}
		}
	}

	std::unordered_set<ResourceDependency> Graphics::determineDependencies() {
		auto dependencies{ LunarMagicInsertable::determineDependencies() };
		const auto folder_dependencies{
			getResourceDependenciesFor(project_graphics_folder_path, Policy::REINSERT)
		};
		dependencies.insert(folder_dependencies.begin(), folder_dependencies.end());
		return dependencies;
	}

	void Graphics::insert() {
		if (!fs::exists(project_graphics_folder_path)) {
			throw InsertionException(fmt::format("No Graphics folder found at {}", project_graphics_folder_path.string()));
		}

		createTemporaryGraphicsFolder();

		spdlog::info("Inserting Graphics");
		spdlog::debug(fmt::format(
			"Inserting Graphics from folder {} into temporary ROM {}",
			project_graphics_folder_path.string(),
			temporary_rom_path.string()
		));

		const auto exit_code{ callLunarMagic("-ImportGFX", temporary_rom_path.string())};

		deleteTemporaryGraphicsFolder();

		if (exit_code == 0) {
			spdlog::info("Successfully inserted Graphics!");
			spdlog::debug(fmt::format(
				"Successfully inserted Graphics from folder {} into temporary ROM {}",
				project_graphics_folder_path.string(),
				temporary_rom_path.string()
			));
		}
		else {
			throw InsertionException(fmt::format(
				"Failed to insert Graphics from folder {} into temporary ROM {}",
				project_graphics_folder_path.string(),
				temporary_rom_path.string()
			));
		}
	}
}