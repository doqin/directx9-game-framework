#pragma once
#include <deque>
#include <memory>
#include <mutex>
#include "DX9GFICommand.h"
#include "DX9GFIGameObject.h"
#include "DX9GFConcurrentCommand.h"

namespace DX9GF {
	class CommandBuffer {
	private:
		std::mutex mutex;
		std::deque<std::shared_ptr<ICommand>> queue;
		std::shared_ptr<ICommand> currentCommand;
	protected:
	public:
		void PushCommand(std::shared_ptr<ICommand> command);
		void StackCommand(std::shared_ptr<ICommand> command);
		void Update(unsigned long long deltaTime);
		void Clear();
		bool IsBusy();
	};
}