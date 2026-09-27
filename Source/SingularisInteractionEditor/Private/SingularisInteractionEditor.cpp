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

#include "Factories/SingularisInteractionBehaviorStrategyFactory.h"
#include "Factories/SingularisInteractionQueryerFactory.h"
#include "Factories/SingularisInteractionStrategyFactory.h"
#include "Factories/SingularisInteractionWidgetFactory.h"
#include "Factories/SingularisInteractorWidgetFactory.h"

// 定义 LOCTEXT_NAMESPACE，用于本地化支持
#define LOCTEXT_NAMESPACE "FSingularisInteractionEditorModule"

void FSingularisInteractionEditorModule::StartupModule()
{
	// 1. 获取 AssetTools 模块
	// 使用 LoadModuleChecked 确保模块存在，如果 AssetTools 没加载，这里崩溃是正常的（依赖项缺失）
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// 2. 注册自定义主分类 (My Plugin / Singularis)
	// 使用 LOCTEXT 进行本地化，这样以后可以翻译成中文或其他语言
	const EAssetTypeCategories::Type SingularisPluginCategory = AssetTools.RegisterAdvancedAssetCategory(
		FName("Singularis"),
		LOCTEXT("SingularisCategory", "Singularis")
	);

	// 3. 注册资产行为 (使用辅助函数，代码更整洁)
	// 这里演示如何注册多个资产，如果你有新资产，只需要复制这一行并替换类名
	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAssetTypeActions_SingularisInteractionQueryer(SingularisPluginCategory))
	);

	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAssetTypeActions_SingularisInteractionStrategy(SingularisPluginCategory))
	);

	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAssetTypeActions_SingularisInteractionBehaviorStrategy(SingularisPluginCategory))
	);

	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAssetTypeActions_SingularisInteractionWidget(SingularisPluginCategory))
	);

	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAssetTypeActions_SingularisInteractorWidget(SingularisPluginCategory))
	);

	// 示例：如果有第二个资产
	// RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_MySecondAsset(MyPluginCategory)));
}

void FSingularisInteractionEditorModule::ShutdownModule()
{
	// 关键点：安全卸载逻辑
	// 1. 检查 AssetTools 模块是否还加载着（编辑器关闭时可能已经被卸载了）
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

		// 2. 遍历数组，注销每一个 Action
		for (auto Action : CreatedAssetTypeActions)
			AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
	}

	// 3. 清空数组，释放智能指针
	CreatedAssetTypeActions.Empty();
}

// 辅助函数实现
void FSingularisInteractionEditorModule::RegisterAssetTypeAction(
	IAssetTools& AssetTools,
	const TSharedRef<IAssetTypeActions>& Action
)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action); // 加入缓存列表
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSingularisInteractionEditorModule, SingularisInteractionEditor)
