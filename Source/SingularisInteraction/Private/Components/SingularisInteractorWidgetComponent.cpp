/* ====================================================================== *
 * SingularisInteractorWidgetComponent.cpp                                *
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

#include "Components/SingularisInteractorWidgetComponent.h"

#include <Blueprint/UserWidget.h>
#include <GameFramework/PlayerController.h>
#include <UObject/ConstructorHelpers.h>

#include "Components/SingularisInteractionComponent.h"
#include "Components/SingularisInteractorComponent.h"

USingularisInteractorWidgetComponent::USingularisInteractorWidgetComponent()
{
	SetIsReplicatedByDefault(false);

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	bAutoActivate = true;

	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(
		TEXT(
			"/SingularisInteraction/UserInterfaces/WBP_Default_SingularisInteractorWidget.WBP_Default_SingularisInteractorWidget_C"
		)
	);

	if (WidgetClassFinder.Succeeded())
		InteractorWidgetClass = WidgetClassFinder.Class;
}

void USingularisInteractorWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(
		GetOwner()->IsA<APlayerController>(),
		TEXT("SingularisInteractorWidgetComponent: Owner is not PlayerController")
	);

	OwnerPlayerController = Cast<APlayerController>(GetOwner());

	// 1) 自动创建视图
	if (bAutoCreateView)
		CreateInteractorView();

	// 2) 绑定交互者组件并推送一次全量状态
	ObserveInteractorComponent();
}

void USingularisInteractorWidgetComponent::SetInteractorView(
	const TScriptInterface<ISingularisInteractorViewInterface>& NewInteractorView
)
{
	// 1) 自动创建视图时视图由组件自身托管，拒绝外部注入
	if (bAutoCreateView) return;

	// 2) 幂等检查，视图未变更时无需重复推送
	if (InteractorView == NewInteractorView) return;
	InteractorView = NewInteractorView;

	// 3) 外部注入后主动拉取一次全量状态，消除错过事件导致的空白期
	if (!OwnerPlayerController.IsValid()) return;

	FullPull(OwnerPlayerController->FindComponentByClass<USingularisInteractorComponent>());
}

void USingularisInteractorWidgetComponent::HandleTargetChanged(
	USingularisInteractionComponent* OldTarget,
	USingularisInteractionComponent* NewTarget
) const
{
	if (!IsValid(InteractorView.GetObject())) return;

	ISingularisInteractorViewInterface::Execute_OnTargetChanged(
		InteractorView.GetObject(),
		OldTarget,
		NewTarget
	);
}

void USingularisInteractorWidgetComponent::HandleTriggered(
	USingularisInteractionComponent* Target,
	const FGameplayTag StrategyTag
) const
{
	if (!IsValid(InteractorView.GetObject())) return;

	ISingularisInteractorViewInterface::Execute_OnTriggered(InteractorView.GetObject(), Target, StrategyTag);
}

void USingularisInteractorWidgetComponent::CreateInteractorView()
{
	// 1) 本地玩家检查
	if (!OwnerPlayerController.IsValid() || !OwnerPlayerController->IsLocalController()) return;

	// 2) 创建交互者控件
	UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(OwnerPlayerController.Get(), InteractorWidgetClass);
	if (!IsValid(CreatedWidget)) return;

	// MustImplement 仅约束编辑器选择器，C++ 与蓝图图赋值可绕过，创建后运行时复核接口实现
	if (!ensureMsgf(
		CreatedWidget->Implements<USingularisInteractorViewInterface>(),
		TEXT("[%s] CreateInteractorView：控件类 %s 未实现 SingularisInteractorViewInterface"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(InteractorWidgetClass.Get())
	))
		return;

	// 3) 缓存视图并添加到视口
	InteractorView = CreatedWidget;
	CreatedWidget->AddToViewport();
}

void USingularisInteractorWidgetComponent::ObserveInteractorComponent()
{
	// 1) 本地玩家检查
	if (!OwnerPlayerController.IsValid() || !OwnerPlayerController->IsLocalController()) return;

	// 2) 获取同属主的交互者组件
	USingularisInteractorComponent* InteractorComponent =
		OwnerPlayerController->FindComponentByClass<USingularisInteractorComponent>();
	if (!IsValid(InteractorComponent)) return;

	// 3) 绑定交互者事件
	InteractorComponent->OnInteractionTargetChangedEvent.AddDynamic(
		this,
		&USingularisInteractorWidgetComponent::HandleTargetChanged
	);
	InteractorComponent->OnInteractionTriggeredEvent.AddDynamic(
		this,
		&USingularisInteractorWidgetComponent::HandleTriggered
	);

	// 4) 绑定后主动拉取一次全量状态，消除错过事件导致的空白期
	FullPull(InteractorComponent);
}

void USingularisInteractorWidgetComponent::FullPull(
	const USingularisInteractorComponent* InteractorComponent
) const
{
	if (!IsValid(InteractorView.GetObject()) || !IsValid(InteractorComponent)) return;

	ISingularisInteractorViewInterface::Execute_OnRefresh(
		InteractorView.GetObject(),
		InteractorComponent->GetCurrentTarget()
	);
}
