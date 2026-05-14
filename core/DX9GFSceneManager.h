#pragma once
#include <vector>

namespace DX9GF {
	class IScene;

	constexpr auto NO_INDEX = -1;

	class SceneManager final {
	private:
		int index = NO_INDEX;
		std::vector<IScene*> scenes;
	protected:
		void Dispose();
	public:
		~SceneManager();
		int GetIndex() const { return index; }
		IScene* GetCurrentScene();
		IScene* GetScene(size_t index);
		void ChangeScene(IScene*);
		void PushScene(IScene*);
		void InsertScene(size_t index, IScene* scene);
		void PopScene();
		void RemoveScene(size_t index);
		void Update(unsigned long long deltaTime);
		void Draw(unsigned long long deltaTime);
		void OnResize(int width, int height);
		void GoToNext();
		void GoToPrevious();
		void GoToScene(size_t index);
		size_t GetSceneCount() const;
	};
}