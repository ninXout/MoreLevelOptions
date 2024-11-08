#include <Geode/Geode.hpp>
#include <iostream>
#include <fstream>

using namespace geode::prelude;

struct ExtraLevelData {
	bool m_holdLevel = false;
	bool m_noNewBest = false;
	bool m_noGravityEffect = false;
};

template <>
struct matjson::Serialize<ExtraLevelData> {
	#define JSON_SERIALIZE(thing) .m_##thing = value[#thing].as<bool>(),
    static ExtraLevelData from_json(const matjson::Value& value) {
        return ExtraLevelData {
            JSON_SERIALIZE(holdLevel)
			JSON_SERIALIZE(noNewBest)
			JSON_SERIALIZE(noGravityEffect)
        };
    }
	#undef JSON_SERIALIZE
	#define JSON_SERIALIZE(thing) { #thing, value.m_##thing },
    static matjson::Value to_json(const ExtraLevelData& value) {
        return matjson::Object {
            JSON_SERIALIZE(holdLevel)
			JSON_SERIALIZE(noNewBest)
			JSON_SERIALIZE(noGravityEffect)
        };
    }

	static bool is_json(const matjson::Value& value) {
		return true;
	}
};

#include <Geode/modify/GJGameLevel.hpp>
class $modify(MLOGameLevel, GJGameLevel) {
	struct Fields {
		ExtraLevelData m_extraData;
	};
};

#include <Geode/modify/PlayLayer.hpp>
class $modify(PlayLayer) {
	void playGravityEffect(bool b) {
		if (!static_cast<MLOGameLevel*>(m_level)->m_fields->m_extraData.m_noGravityEffect) PlayLayer::playGravityEffect(b);
	}

	void showNewBest(bool p0, int p1, int p2, bool p3, bool p4, bool p5) {
		if (!static_cast<MLOGameLevel*>(m_level)->m_fields->m_extraData.m_noNewBest) PlayLayer::showNewBest(p0, p1, p2, p3, p4, p5);
		else PlayLayer::delayedResetLevel();
	}

	void startGame() {
		PlayLayer::startGame();
		if (static_cast<MLOGameLevel*>(m_level)->m_fields->m_extraData.m_holdLevel) {
			handleButton(true, (int)PlayerButton::Jump, true);
			handleButton(true, (int)PlayerButton::Jump, false);
		}
	}
};

#define MLO_TOGGLE(tog) static_cast<MLOGameLevel*>(m_level)->m_fields->m_extraData.tog = !static_cast<MLOGameLevel*>(m_level)->m_fields->m_extraData.tog;

#include <Geode/modify/GameLevelOptionsLayer.hpp>
class $modify(GameLevelOptionsLayer) {
	static void onModify(auto& self) {
        if (!self.setHookPriority("GameLevelOptionsLayer::setupOptions", -9999999)) {
            geode::log::warn("Failed to set hook priority for GameLevelOptionsLayer::setupOptions");
        }
    }

	void setupOptions() {
		auto winSize = CCDirector::get()->getWinSize();

		// title texts
		CCLabelBMFont* levelSpec = CCLabelBMFont::create("Level Settings", "goldFont.fnt");
		levelSpec->setPosition(ccp(winSize.width / 2.f, winSize.height - 40.f));
		levelSpec->setScale(0.85f);
		CCLabelBMFont* gameSpec = CCLabelBMFont::create("Game Settings", "goldFont.fnt");
		gameSpec->setPosition(ccp(winSize.width / 2.f, winSize.height - 40.f));
		gameSpec->setScale(0.85f);

		// loading data
		if (Mod::get()->hasSavedValue(std::to_string(m_level->m_levelID))) {
			static_cast<MLOGameLevel*>(m_level)->m_fields->m_extraData = Mod::get()->getSavedValue<ExtraLevelData>(std::to_string(m_level->m_levelID));
		}

		// page 1
		layerForPage(0)->addChild(levelSpec, 5);

		addToggle("Low Detail Mode", 1, m_level->m_lowDetailModeToggled, "Toggles off all objects marked as High Detail.");
		addToggle("Disable Shake", 2, m_level->m_disableShakeToggled, "Disables all shake triggers in the level.");
		addToggle("Hold Level", 3, static_cast<MLOGameLevel*>(m_level)->m_fields->m_extraData.m_holdLevel, "Automatically holds the player button while playing the level. Used for hold auto levels.");
		addToggle("No New Best", 4, static_cast<MLOGameLevel*>(m_level)->m_fields->m_extraData.m_noNewBest, "Stops the New Best notification from showing up on new best percentages.");
		addToggle("No Gravity Effect", 5, static_cast<MLOGameLevel*>(m_level)->m_fields->m_extraData.m_noGravityEffect, "Stops the gravity portal effect from showing.");

		offsetToNextPage();

		// page 2
		layerForPage(1)->addChild(gameSpec, 5);

		addGVToggle("Flip 2 player controls", "0010", "Flips the 2 player controls.");
		addGVToggle("Fast Respawn", "0052", "Shortens the respawn time from 1.0s to 0.5s.");
		addGVToggle("Disable Orb Scale", "0140", "Disable scaling effect when hitting an orb.");
		addGVToggle("Hide Attempts", "0135", "Hide the attempt counter.");
		addGVToggle("Hide Attempts in Practice", "0134", "Hide the attempt counter in practice mode.");
	}

	void didToggle(int opt) {
		switch (opt) {
			case 3:
				MLO_TOGGLE(m_holdLevel);
				break;
			case 4:
				MLO_TOGGLE(m_noNewBest);
				break;
			case 5:
				MLO_TOGGLE(m_noGravityEffect);
				break;
			default:
				return GameLevelOptionsLayer::didToggle(opt);
		}

		if (m_level->m_levelID > 0) {
			ExtraLevelData data = static_cast<MLOGameLevel*>(m_level)->m_fields->m_extraData;

			if (!data.m_holdLevel && !data.m_noNewBest && !data.m_noGravityEffect) { // TODO: make this a map or something so i can do this easier
				Mod::get()->getSaveContainer().erase(std::to_string(m_level->m_levelID));
			} else {
				Mod::get()->setSavedValue<ExtraLevelData>(std::to_string(m_level->m_levelID), data);
			}
		}
	}
};

#undef MLO_TOGGLE