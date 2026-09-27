/* ====================================================================== *
 * SingularisInteractorWidgetComponent.h                                  *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/09/27 | Author: TrifingZW                                *
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
#include <GameplayTagContainer.h>
#include <Components/ActorComponent.h>
#include <UObject/ScriptInterface.h>

#include "Interfaces/SingularisInteractorViewInterface.h"
#include "SingularisInteractorWidgetComponent.generated.h"

class APlayerController;
class UUserWidget;
class USingularisInteractionComponent;
class USingularisInteractorComponent;

/**
 * 引力奇点交互者控件组件。
 *
 * 玩家侧交互观察者：实例化交互者视图，订阅交互者组件的目标变更与交互触发事件，
 * 将交互者状态与事件经 ISingularisInteractorViewInterface（SPI）推送至视图。
 * 组件仅依赖接口而非具体控件类型，任意实现该接口的 UObject 均可作为视图接入。
 */
UCLASS(
	Blueprintable,
	BlueprintType,
	ClassGroup = ("Singularis"),
	meta = (BlueprintSpawnableComponent, DisplayName = "引力奇点交互者控件组件")
)
class SINGULARISINTERACTION_API USingularisInteractorWidgetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
#pragma region Parameter

	/** 自动创建视图 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互者控件组件",
		meta = (DisplayName = "自动创建视图")
	)
	bool bAutoCreateView = true;

	/** 交互者控件类 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "引力奇点交互者控件组件",
		meta = (
			DisplayName = "交互者控件类",
			MustImplement = "/Script/SingularisInteraction.SingularisInteractorViewInterface",
			EditCondition = "bAutoCreateView"
		)
	)
	TSubclassOf<UUserWidget> InteractorWidgetClass = nullptr;

#pragma endregion

private:
#pragma region State

	/** 拥有本组件的玩家控制器 */
	TWeakObjectPtr<APlayerController> OwnerPlayerController = nullptr;

	/** 运行时实例化的交互者视图缓存 */
	UPROPERTY(Transient)
	TScriptInterface<ISingularisInteractorViewInterface> InteractorView{};

#pragma endregion

public:
#pragma region Constructors

	USingularisInteractorWidgetComponent();

#pragma endregion

#pragma region ActorComponent Interface

	virtual void BeginPlay() override;

#pragma endregion

#pragma region API

	/**
	 * 设置交互者视图。
	 *
	 * 仅在关闭自动创建视图时用于外部注入，设置后立即推送一次全量状态。
	 *
	 * @param NewInteractorView 交互者视图对象。
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "引力奇点交互者控件组件|API",
		meta = (DisplayName = "设置交互者视图")
	)
	void SetInteractorView(const TScriptInterface<ISingularisInteractorViewInterface>& NewInteractorView);

	/**
	 * 获取运行时实例化的交互者视图。
	 *
	 * @return 交互者视图对象；尚未实例化时返回 nullptr。
	 */
	UFUNCTION(
		BlueprintPure,
		Category = "引力奇点交互者控件组件|API",
		meta = (DisplayName = "GetInteractorView")
	)
	UObject* GetInteractorView() const { return InteractorView.GetObject(); }

#pragma endregion

private:
#pragma region Callback

	/** 交互目标变更回调：转发至视图 */
	UFUNCTION()
	void HandleTargetChanged(
		USingularisInteractionComponent* OldTarget,
		USingularisInteractionComponent* NewTarget
	) const;

	/** 交互触发回调：转发至视图 */
	UFUNCTION()
	void HandleTriggered(USingularisInteractionComponent* Target, FGameplayTag StrategyTag) const;

#pragma endregion

#pragma region Internal Function

	/** 在本地客户端实例化交互者视图并添加到视口 */
	void CreateInteractorView();

	/** 绑定交互者组件事件并推送一次全量状态 */
	void ObserveInteractorComponent();

	/** 推送当前锁定的交互目标至视图 */
	void FullPull(const USingularisInteractorComponent* InteractorComponent) const;

#pragma endregion
};
