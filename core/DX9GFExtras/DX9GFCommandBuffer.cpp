#include "pch.h"
#include "DX9GFCommandBuffer.h"

void DX9GF::CommandBuffer::PushCommand(std::shared_ptr<ICommand> command)
{
	{
		std::scoped_lock<std::mutex> lock{mutex};
		this->queue.push_back(command);
	}
}

void DX9GF::CommandBuffer::StackCommand(std::shared_ptr<ICommand> command)
{
	std::scoped_lock<std::mutex> lock{ mutex };
	if (this->queue.empty()) {
		this->queue.push_back(command);
		return;
	}
	auto lastCommand = this->queue.back();
	auto concurrent = std::dynamic_pointer_cast<ConcurrentCommand>(lastCommand);
	if (concurrent) {
		concurrent->AddCommand(command);
	}
	else {
		auto newConcurrent = std::make_shared<ConcurrentCommand>();
		newConcurrent->AddCommand(lastCommand);
		newConcurrent->AddCommand(command);
		this->queue.back() = newConcurrent;
	}
}

void DX9GF::CommandBuffer::Update(unsigned long long deltaTime)
{
	if (currentCommand && currentCommand->IsFinished()) {
		currentCommand.reset();
	}

	if (!currentCommand) {
		if (queue.empty()) {
			return;
		}

		currentCommand = queue.front();
		queue.pop_front();
	}

	if (!currentCommand || currentCommand->IsFinished()) {
		currentCommand.reset();
		return;
	}

	currentCommand->Execute(deltaTime);
}

void DX9GF::CommandBuffer::Clear()
{
	queue.clear();
	currentCommand.reset();
}

bool DX9GF::CommandBuffer::IsBusy()
{
	std::scoped_lock<std::mutex> lock{ mutex };
	return currentCommand != nullptr || !queue.empty();
}
