#pragma once

#include "Modules/ModuleManager.h"

class FSingularisInteractionModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
