/* ====================================================================== *
 * SingularisInteractionWidgetComponent.cpp                               *
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

#include "Components/SingularisInteractionWidgetComponent.h"

#include <Blueprint/UserWidget.h>
#include <Components/ShapeComponent.h>
#include <Components/WidgetComponent.h>
#include <GameFramework/Pawn.h>
#include <GameFramework/PlayerController.h>
#include <UObject/ConstructorHelpers.h>

#include "SingularisInteraction.h"
#include "Components/SingularisInteractionComponent.h"
#include "Components/SingularisInteractorComponent.h"

#define ECC_INTERACTION ECC_GameTraceChannel1

USingularisInteractionWidgetComponent::USingularisInteractionWidgetComponent()
{
	SetIsReplicatedByDefault(false);

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	bAutoActivate = true;

	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(
		TEXT(
			"/SingularisInteraction/UserInterfaces/WBP_Default_SingularisInteractionWidget.WBP_Default_SingularisInteractionWidget_C"
		)
	);

	if (WidgetClassFinder.Succeeded())
		InteractionWidgetClass = WidgetClassFinder.Class;
	else
		UE_LOG(
		LogSingularisInteraction,
		Error,
		TEXT("默认交互控件类加载失败：%s"),
		TEXT("/SingularisInteraction/UserInterfaces/WBP_Default_SingularisInteractionWidget")
	);
}

void USingularisInteractionWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1) 实例化交互视图并挂载到控件组件
	ProxyWidgetComponent();

	// 2) 装配提示范围的重叠回调
	ProxyPromptVolume();

	// 3) 订阅交互组件事件并推送一次全量状态
	ObserveInteractionComponent();
}

void USingularisInteractionWidgetComponent::HandleInteraction() const
{
	if (!IsValid(InteractionView.GetObject())) return;

	ISingularisInteractionViewInterface::Execute_OnTrigger(InteractionView.GetObject());
}

void USingularisInteractionWidgetComponent::HandleHover() const
{
	if (!IsValid(InteractionView.GetObject())) return;

	ISingularisInteractionViewInterface::Execute_OnHover(InteractionView.GetObject());
}

void USingularisInteractionWidgetComponent::HandleUnhover() const
{
	if (!IsValid(InteractionView.GetObject())) return;

	ISingularisInteractionViewInterface::Execute_OnUnhover(InteractionView.GetObject());
}

void USingularisInteractionWidgetComponent::ProxyWidgetComponent()
{
	// 1) 本地玩家检查
	const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController())
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] ProxyWidgetComponent：非本地客户端，跳过控件创建"),
			*GetNameSafe(GetOwner())
		);
		return;
	}

	// 2) 获取承载交互控件的控件组件
	UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(WidgetComponentReference.GetComponent(GetOwner()));
	if (!IsValid(WidgetComponent))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] ProxyWidgetComponent：控件组件引用无效，无法挂载交互控件"),
			*GetNameSafe(GetOwner())
		);
		return;
	}

	// 3) 创建交互控件
	UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(GetWorld(), InteractionWidgetClass);
	if (!IsValid(CreatedWidget))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] ProxyWidgetComponent：控件类 %s 创建控件失败"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(InteractionWidgetClass.Get())
		);
		return;
	}

	// MustImplement 仅约束编辑器选择器，C++ 与蓝图图赋值可绕过，创建后运行时复核接口实现
	if (!ensureMsgf(
		CreatedWidget->Implements<USingularisInteractionViewInterface>(),
		TEXT("[%s] ProxyWidgetComponent：控件类 %s 未实现 SingularisInteractionViewInterface"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(InteractionWidgetClass.Get())
	))
		return;

	// 4) 装配控件至控件组件，按屏幕空间展示且不参与碰撞
	InteractionView = CreatedWidget;

	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetWidget(CreatedWidget);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] ProxyWidgetComponent：交互控件 %s 创建并挂载成功"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(CreatedWidget)
	);
}

void USingularisInteractionWidgetComponent::ProxyPromptVolume()
{
	// 1) 本地玩家检查
	const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;

	// 2) 获取提示范围组件
	UShapeComponent* PromptVolume = Cast<UShapeComponent>(PromptVolumeReference.GetComponent(GetOwner()));
	if (!IsValid(PromptVolume)) return;

	// 3) 提示范围仅用于重叠反馈，忽略交互通道避免遮挡视线查询
	PromptVolume->SetCollisionResponseToChannel(ECC_INTERACTION, ECR_Ignore);

	// 4) 装配重叠回调
	PromptVolume->OnComponentBeginOverlap.AddDynamic(
		this,
		&USingularisInteractionWidgetComponent::OnPromptVolumeBeginOverlap
	);
	PromptVolume->OnComponentEndOverlap.AddDynamic(
		this,
		&USingularisInteractionWidgetComponent::OnPromptVolumeEndOverlap
	);

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] ProxyPromptVolume：提示范围 %s 装配完成"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(PromptVolume)
	);
}

void USingularisInteractionWidgetComponent::ObserveInteractionComponent()
{
	// 1) 本地玩家检查
	const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;

	// 2) 获取关联的交互组件
	USingularisInteractionComponent* InteractionComponent = Cast<USingularisInteractionComponent>(
		InteractionComponentReference.GetComponent(GetOwner())
	);
	if (!IsValid(InteractionComponent))
	{
		UE_LOG(
			LogSingularisInteraction,
			Warning,
			TEXT("[%s] ObserveInteractionComponent：交互组件引用无效，无法订阅事件"),
			*GetNameSafe(GetOwner())
		);
		return;
	}

	// 3) 订阅交互状态事件
	InteractionComponent->OnInteractionEvent.AddDynamic(
		this,
		&USingularisInteractionWidgetComponent::HandleInteraction
	);
	InteractionComponent->OnInteractionHoverEvent.AddDynamic(
		this,
		&USingularisInteractionWidgetComponent::HandleHover
	);
	InteractionComponent->OnInteractionUnhoverEvent.AddDynamic(
		this,
		&USingularisInteractionWidgetComponent::HandleUnhover
	);

	// 4) 绑定后主动拉取一次全量状态，消除错过事件导致的空白期
	if (!IsValid(InteractionView.GetObject())) return;

	ISingularisInteractionViewInterface::Execute_OnRefresh(
		InteractionView.GetObject(),
		InteractionComponent->Enabled(),
		InteractionComponent->Hovered()
	);

	UE_LOG(
		LogSingularisInteraction,
		Display,
		TEXT("[%s] ObserveInteractionComponent：已绑定交互组件 %s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(InteractionComponent)
	);
}

// ReSharper disable CppMemberFunctionMayBeConst

void USingularisInteractionWidgetComponent::OnPromptVolumeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	// 仅响应挂载本地交互者组件的玩家 Pawn，避免无关 Actor 触发提示反馈
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!IsValid(Pawn)) return;

	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;

	const USingularisInteractorComponent* InteractorComponent =
		PlayerController->FindComponentByClass<USingularisInteractorComponent>();
	if (!IsValid(InteractorComponent)) return;

	if (!IsValid(InteractionView.GetObject())) return;

	ISingularisInteractionViewInterface::Execute_OnEnterRange(InteractionView.GetObject());
}

void USingularisInteractionWidgetComponent::OnPromptVolumeEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	// 仅响应挂载本地交互者组件的玩家 Pawn，避免无关 Actor 触发提示反馈
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!IsValid(Pawn)) return;

	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;

	const USingularisInteractorComponent* InteractorComponent =
		PlayerController->FindComponentByClass<USingularisInteractorComponent>();
	if (!IsValid(InteractorComponent)) return;

	if (!IsValid(InteractionView.GetObject())) return;

	ISingularisInteractionViewInterface::Execute_OnExitRange(InteractionView.GetObject());
}

// ReSharper restore CppMemberFunctionMayBeConst
