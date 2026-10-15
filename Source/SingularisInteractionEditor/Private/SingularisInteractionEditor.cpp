/* ====================================================================== *
 * SingularisInteractionEditor.cpp                                        *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/01/24 | Author: TrifingZW                                *
 * Licensed under MIT License                                             *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   *
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   *
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      *
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 *
 * ====================================================================== */

#include "SingularisInteractionEditor.h"

#include <AssetToolsModule.h>
#include <IAssetTools.h>

#include "Factories/SingularisInteractionBehaviorFactory.h"
#include "Factories/SingularisInteractionQueryerFactory.h"
#include "Factories/SingularisInteractionResponseFactory.h"
#include "Factories/SingularisInteractionSessionFactory.h"
#include "Factories/SingularisInteractionWidgetFactory.h"
#include "Factories/SingularisInteractorWidgetFactory.h"
#include "Nodes/SingularisInteractionBlueprintCompileHooks.h"
#include "Styles/SingularisInteractionEditorStyle.h"

DEFINE_LOG_CATEGORY(LogSingularisInteractionEditor);

#define LOCTEXT_NAMESPACE "FSingularisInteractionEditorModule"

void FSingularisInteractionEditorModule::StartupModule()
{
	// 1) 登记 Singularis 资产分类
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	const EAssetTypeCategories::Type SingularisPluginCategory = AssetTools.RegisterAdvancedAssetCategory(
		FName("Singularis"),
		LOCTEXT("SingularisCategory", "Singularis")
	);

	// 2) 登记六类资产行为
	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAssetTypeActions_SingularisInteractionQueryer(SingularisPluginCategory))
	);

	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAssetTypeActions_SingularisInteractionSession(SingularisPluginCategory))
	);

	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAssetTypeActions_SingularisInteractionResponse(SingularisPluginCategory))
	);

	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAssetTypeActions_SingularisInteractionBehavior(SingularisPluginCategory))
	);

	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAssetTypeActions_SingularisInteractionWidget(SingularisPluginCategory))
	);

	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAssetTypeActions_SingularisInteractorWidget(SingularisPluginCategory))
	);

	// 3) 登记蓝图编译 hook：把「会话事件」声明回填进响应类 CDO
	FSingularisInteractionBlueprintCompileHook::Register(BlueprintCompileHandle);

	// 4) 注册 Slate 样式集：K2 节点图标槽位
	FSingularisInteractionEditorStyle::Register();

	UE_LOG(
		LogSingularisInteractionEditor,
		Display,
		TEXT("StartupModule：编辑器模块初始化完成，已登记 %d 项资产类型行为与蓝图编译 hook"),
		CreatedAssetTypeActions.Num()
	);
}

void FSingularisInteractionEditorModule::ShutdownModule()
{
	// 1) 先反注册 Slate 样式集与蓝图编译 hook（不依赖 AssetTools 存活状态）
	FSingularisInteractionEditorStyle::Unregister();
	FSingularisInteractionBlueprintCompileHook::Unregister(BlueprintCompileHandle);

	// 2) 编辑器关闭时 AssetTools 可能已卸载，需先检查再反注册
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

		for (const auto& Action : CreatedAssetTypeActions)
			AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
	}

	UE_LOG(
		LogSingularisInteractionEditor,
		Display,
		TEXT("ShutdownModule：编辑器模块卸载，已反注册 %d 项资产类型行为"),
		CreatedAssetTypeActions.Num()
	);

	CreatedAssetTypeActions.Empty();
}

void FSingularisInteractionEditorModule::RegisterAssetTypeAction(
	IAssetTools& AssetTools,
	const TSharedRef<IAssetTypeActions>& Action
)
{
	// 1) 登记资产行为并留存引用，供卸载时反注册
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSingularisInteractionEditorModule, SingularisInteractionEditor)
