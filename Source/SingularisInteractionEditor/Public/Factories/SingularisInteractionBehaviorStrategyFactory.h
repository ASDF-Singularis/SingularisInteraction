/* ====================================================================== *
 * SingularisInteractionBehaviorStrategyFactory.h                         *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/01/30 | Author: TrifingZW                                *
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

#pragma once

#include <CoreMinimal.h>
#include <AssetTypeActions/AssetTypeActions_Blueprint.h>
#include <Factories/Factory.h>
#include <Objects/SingularisInteractionBehaviorStrategy.h>

#include "SingularisInteractionBehaviorStrategyFactory.generated.h"

/**
 * 引力奇点交互行为策略工厂。
 *
 * 在内容浏览器中创建 USingularisInteractionBehaviorStrategy 的蓝图资产。
 */
UCLASS()
class SINGULARISINTERACTIONEDITOR_API USingularisInteractionBehaviorStrategyFactory : public UFactory
{
	GENERATED_BODY()

public:
	USingularisInteractionBehaviorStrategyFactory();
	virtual UObject* FactoryCreateNew(
		UClass* InClass,
		UObject* InParent,
		FName InName,
		EObjectFlags Flags,
		UObject* Context,
		FFeedbackContext* Warn
	) override;
	virtual bool ShouldShowInNewMenu() const override;
};

/**
 * 引力奇点交互行为策略资产行为。
 */
class FAssetTypeActions_SingularisInteractionBehaviorStrategy : public FAssetTypeActions_Blueprint
{
public:
	explicit FAssetTypeActions_SingularisInteractionBehaviorStrategy(const EAssetTypeCategories::Type InAssetCategory)
		: AssetTypeCategory(InAssetCategory) {}

	virtual FText GetName() const override
	{
		return NSLOCTEXT(
			"SingularisInteractionEditor",
			"AssetTypeActions_SingularisInteractionBehaviorStrategy",
			"Singularis Interaction Behavior Strategy"
		);
	}

	virtual FColor GetTypeColor() const override { return FColor(63, 126, 255); }

	virtual UClass* GetSupportedClass() const override { return USingularisInteractionBehaviorStrategy::StaticClass(); }

	virtual UFactory* GetFactoryForBlueprintType(UBlueprint* InBlueprint) const override
	{
		// 1) 动态实例化工厂对象以接管该资产蓝图的创建流程
		USingularisInteractionBehaviorStrategyFactory* Factory = NewObject<
			USingularisInteractionBehaviorStrategyFactory>();
		return Factory;
	}

	virtual uint32 GetCategories() override { return AssetTypeCategory; }

	virtual const TArray<FText>& GetSubMenus() const override
	{
		// 1) 将资产收纳至右键菜单的指定子目录中
		static const TArray SubMenus = {
			FText::FromString("SingularisInteraction"),
		};

		return SubMenus;
	}

private:
	EAssetTypeCategories::Type AssetTypeCategory;
};
