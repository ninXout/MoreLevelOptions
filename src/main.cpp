#include <Geode/Geode.hpp>
#include <iostream>
#include <fstream>
#include <ninxout.options_api/include/API.hpp>

using namespace geode::prelude;

struct ExtraLevelData {
	bool m_holdLevel = false;
	bool m_noNewBest = false;
	bool m_noGravityEffect = false;
};

template <>
struct matjson::Serialize<ExtraLevelData> {
	#define JSON_SERIALIZE(thing) .m_##thing = value[#thing].as<bool>().unwrapOrDefault(),
    static Result<ExtraLevelData> fromJson(const matjson::Value& value) {
		auto data = ExtraLevelData {
            JSON_SERIALIZE(holdLevel)
			JSON_SERIALIZE(noNewBest)
			JSON_SERIALIZE(noGravityEffect)
        };
        return Ok(data);
    }
	#undef JSON_SERIALIZE
	#define JSON_SERIALIZE(thing) { #thing, value.m_##thing },
    static matjson::Value toJson(const ExtraLevelData& value) {
        return matjson::makeObject({
            JSON_SERIALIZE(holdLevel)
			JSON_SERIALIZE(noNewBest)
			JSON_SERIALIZE(noGravityEffect)
        });
    }
	#undef JSON_SERIALIZE
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

$on_mod(Loaded) {
	OptionsAPI::addPreLevelSetting<bool>(
		"Hold Level", 
		"hold-level"_spr, 
		[](GJGameLevel* lvl) {
			MLOGameLevel* mlo = static_cast<MLOGameLevel*>(lvl);
			mlo->m_fields->m_extraData.m_holdLevel = !mlo->m_fields->m_extraData.m_holdLevel;

			if (lvl->m_levelID > 0) {
				ExtraLevelData data = mlo->m_fields->m_extraData;
	
				if (!data.m_holdLevel && !data.m_noNewBest && !data.m_noGravityEffect) { // TODO: make this a map or something so i can do this easier
					Mod::get()->getSaveContainer().erase(std::to_string(lvl->m_levelID));
				} else {
					Mod::get()->setSavedValue<ExtraLevelData>(std::to_string(lvl->m_levelID), data);
				}
			}
		},
		[](GJGameLevel* lvl) {
			MLOGameLevel* mlo = static_cast<MLOGameLevel*>(lvl);
			return mlo->m_fields->m_extraData.m_holdLevel;
		},
		"Automatically holds the player button while playing the level. Used for hold auto levels."
	);
	OptionsAPI::addPreLevelSetting<bool>(
		"No New Best", 
		"no-new-best"_spr, 
		[](GJGameLevel* lvl) {
			MLOGameLevel* mlo = static_cast<MLOGameLevel*>(lvl);
			mlo->m_fields->m_extraData.m_noNewBest = !mlo->m_fields->m_extraData.m_noNewBest;

			if (lvl->m_levelID > 0) {
				ExtraLevelData data = mlo->m_fields->m_extraData;
	
				if (!data.m_holdLevel && !data.m_noNewBest && !data.m_noGravityEffect) { // TODO: make this a map or something so i can do this easier
					Mod::get()->getSaveContainer().erase(std::to_string(lvl->m_levelID));
				} else {
					Mod::get()->setSavedValue<ExtraLevelData>(std::to_string(lvl->m_levelID), data);
				}
			}
		},
		[](GJGameLevel* lvl) {
			MLOGameLevel* mlo = static_cast<MLOGameLevel*>(lvl);
			return mlo->m_fields->m_extraData.m_noNewBest;
		},
		"Stops the New Best notification from showing up on new best percentages."
	);
	OptionsAPI::addPreLevelSetting<bool>(
		"No Gravity Effect", 
		"no-gravity-fx"_spr, 
		[](GJGameLevel* lvl) {
			MLOGameLevel* mlo = static_cast<MLOGameLevel*>(lvl);
			mlo->m_fields->m_extraData.m_noGravityEffect = !mlo->m_fields->m_extraData.m_noGravityEffect;

			if (lvl->m_levelID > 0) {
				ExtraLevelData data = mlo->m_fields->m_extraData;
	
				if (!data.m_holdLevel && !data.m_noNewBest && !data.m_noGravityEffect) { // TODO: make this a map or something so i can do this easier
					Mod::get()->getSaveContainer().erase(std::to_string(lvl->m_levelID));
				} else {
					Mod::get()->setSavedValue<ExtraLevelData>(std::to_string(lvl->m_levelID), data);
				}
			}
		},
		[](GJGameLevel* lvl) {
			MLOGameLevel* mlo = static_cast<MLOGameLevel*>(lvl);
			return mlo->m_fields->m_extraData.m_noGravityEffect;
		},
		"Stops the gravity portal effect from showing."
	);
};