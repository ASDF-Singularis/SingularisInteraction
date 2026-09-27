#pragma once

#include <CoreMinimal.h>

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSingularisInteraction, Log, All);

/**
 * 引力奇点交互模块。
 *
 * 插件入口：登记日志分类并托管模块生命周期，交互逻辑由组件、策略与子系统承载。
 */
class FSingularisInteractionModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
