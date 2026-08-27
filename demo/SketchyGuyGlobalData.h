#pragma once
#include "DX9GF.h"
#include "DX9GFExtras.h"

namespace Demo {
	// Save-scoped state for the card-pack peddler. He can be placed in more than one scene, so
	// "have we met" and "has the free first pack been used" have to live in one global instance
	// instead of on any single scene - same rationale as PlayerGlobalData.
	class SketchyGuyGlobalData : public DX9GF::ISaveable {
	private:
		bool hasMet = false;        // the intro conversation has played to the end
		bool hasBoughtPack = false; // the first pack (free) has been claimed

		SketchyGuyGlobalData() { Reset(); }
	public:
		static SketchyGuyGlobalData* GetInstance() {
			static SketchyGuyGlobalData instance;
			return &instance;
		}

		void Reset() {
			hasMet = false;
			hasBoughtPack = false;
		}

		bool HasMet() const { return hasMet; }
		void SetMet(bool met) { hasMet = met; }

		bool HasBoughtPack() const { return hasBoughtPack; }
		void SetBoughtPack(bool bought) { hasBoughtPack = bought; }

		// Gold price of a pack: the first one is on the house.
		int PackCost() const { return hasBoughtPack ? 75 : 0; }

		std::string GetSaveID() const override { return "SketchyGuyGlobalData"; }
		void GenerateSaveData(nlohmann::json& outData) override {
			outData["hasMet"] = hasMet;
			outData["hasBoughtPack"] = hasBoughtPack;
		}
		void RestoreSaveData(const nlohmann::json& inData) override {
			if (inData.contains("hasMet")) hasMet = inData["hasMet"];
			if (inData.contains("hasBoughtPack")) hasBoughtPack = inData["hasBoughtPack"];
		}
	};
}
