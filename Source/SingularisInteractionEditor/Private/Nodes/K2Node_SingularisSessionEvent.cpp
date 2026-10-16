/* ====================================================================== *
 * K2Node_SingularisSessionEvent.cpp                                      *
 * ====================================================================== *
 * SPDX-License-Identifier: MIT                                           *
 * SPDX-FileCopyrightText: 2026 TrifingZW <TrifingZW@gmail.com>           *
 *                                                                        *
 * Copyright (c) 2026 TrifingZW. All Rights Reserved.                     *
 * Created: 2026/10/13 | Author: TrifingZW                                *
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

#include "Nodes/K2Node_SingularisSessionEvent.h"

#include <BlueprintActionDatabaseRegistrar.h>
#include <BlueprintNodeSpawner.h>
#include <EdGraphSchema_K2.h>
#include <K2Node_CallFunction.h>
#include <K2Node_FunctionEntry.h>
#include <KismetCompiler.h>
#include <EdGraph/EdGraph.h>
#include <Engine/Blueprint.h>
#include <Kismet2/BlueprintEditorUtils.h>
#include <Kismet2/CompilerResultsLog.h>
#include <UObject/Class.h>

#include "Objects/SingularisInteractionResponse.h"
#include "Styles/SingularisInteractionEditorStyle.h"
#include "Types/SingularisInteractionSessionType.h"

#define LOCTEXT_NAMESPACE "K2Node_SingularisSessionEvent"

namespace
{
	/** 事件函数名前缀：便于在“调用函数”列表与调试器中辨认来源 */
	constexpr auto SessionEventFunctionPrefix = TEXT("SessionEvent_");

	/** 未配置标签时的事件函数名占位（事件名必须非空，否则编译器不会生成 stub 函数） */
	constexpr auto PlaceholderEventFunctionName = TEXT("SessionEvent");

	/**
	 * 标签域过滤元数据键：与受域限标签属性的 Categories 元数据同键，
	 * 标签选择器据此把订阅标签的可选范围限制在该域子树内。
	 */
	constexpr auto TagDomainMetadataKey = TEXT("Categories");

	/** 强制选择元数据键：与受域限标签属性的 ForceSelection 元数据同键，拒绝手填域外标签 */
	constexpr auto TagDomainForceSelectionKey = TEXT("ForceSelection");

	/**
	 * 会话事件的订阅可选域。
	 *
	 * @return 事件域根标签；会话事件词汇恒为事件域根或其子标签（领域事件经子标签开放扩展）。
	 */
	FGameplayTag GetSubscribableEventRoot()
	{
		return FGameplayTag::RequestGameplayTag(FName("Singularis.Interaction.Event"));
	}

	/** 订阅标签 → 合法函数名片段（非字母数字一律折叠为下划线） */
	FString SanitizeTagToNameFragment(const FGameplayTag& EventTag)
	{
		FString Fragment = EventTag.ToString();
		for (TCHAR& Character : Fragment)
		{
			if (!FChar::IsAlnum(Character))
				Character = TEXT('_');
		}

		return Fragment;
	}

	/** 订阅标签 → 结构体导出文本（与引擎标签引脚控件同形态，供配置引脚镜像与解析） */
	FString ExportTagText(const FGameplayTag& EventTag)
	{
		FString ExportedText;
		FGameplayTag::StaticStruct()->ExportText(ExportedText, &EventTag, &EventTag, nullptr, PPF_None, nullptr);
		return ExportedText;
	}
}

UK2Node_SingularisSessionEvent::UK2Node_SingularisSessionEvent()
{
	// 入场默认订阅事件域达成：配置引脚与事件函数名均由其推导，未改动前即为可用声明
	SessionEventTag = FGameplayTag::RequestGameplayTag(FName("Singularis.Interaction.Event.Complete"));
}

FName UK2Node_SingularisSessionEvent::BuildFunctionNameFromTag(const FGameplayTag& EventTag)
{
	// 标签未配置时回退占位名（事件名即 stub 函数名，必须非空）
	if (!EventTag.IsValid())
		return FName(PlaceholderEventFunctionName);

	return FName(*(SessionEventFunctionPrefix + SanitizeTagToNameFragment(EventTag)));
}

void UK2Node_SingularisSessionEvent::PostLoad()
{
	Super::PostLoad();

	// 委托输出引脚只是编译器的入口约定：本节点自身不使用，加载后一律隐藏
	HideDelegateOutputPin();
}

void UK2Node_SingularisSessionEvent::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	// 1) 隐藏委托输出引脚：框架按名直调本事件，节点只暴露「负载 + 配置 + 执行输出」
	HideDelegateOutputPin();

	// 2) 负载输出引脚：会话事件在分派帧内推送，展开期把本引脚的连线改接分派帧读取，
	//    因此不参与事件签名（事件函数保持无参，框架可直调）；结构可拆分，字段经拆分引脚取用
	UEdGraphPin* const PayloadPin = CreatePin(
		EGPD_Output,
		UEdGraphSchema_K2::PC_Struct,
		FSingularisInteractionSessionEvent::StaticStruct(),
		GetPayloadPinName()
	);
	PayloadPin->PinFriendlyName = LOCTEXT("PayloadPin", "会话事件");
	PayloadPin->PinToolTip = LOCTEXT(
		"PayloadPinTip",
		"处理器收到的会话事件：事件标签、归一化量值、开放负载与会话上下文；可拆分取字段"
	).ToString();

	// 3) 订阅标签配置引脚：输入引脚不参与事件签名（编译器剔除全部 EGPD_Input），
	//    因此这里的配置不会被编译成事件参数；未连接时直接在节点上选择，取值限于事件域
	UEdGraphPin* const EventTagPin = CreatePin(
		EGPD_Input,
		UEdGraphSchema_K2::PC_Struct,
		FGameplayTag::StaticStruct(),
		GetEventTagPinName()
	);
	EventTagPin->PinFriendlyName = LOCTEXT("EventTagPin", "事件标签");
	EventTagPin->PinToolTip = FText::Format(
		LOCTEXT(
			"EventTagPinTip",
			"订阅的事件标签，取值限于事件域（{0}）子树；层级匹配：事件标签本身或其子标签均命中"
		),
		FText::FromString(GetSubscribableEventRoot().ToString())
	).ToString();

	// 4) 配置引脚镜像订阅标签（构造期直写，不触发回调与校验；新建节点即默认订阅事件域达成）
	EventTagPin->DefaultValue = ExportTagText(SessionEventTag);
}

void UK2Node_SingularisSessionEvent::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();

	// 事件函数名必须非空，否则编译器不会为本节点生成 stub 函数
	RefreshEventFunctionName();
}

void UK2Node_SingularisSessionEvent::PostPasteNode()
{
	Super::PostPasteNode();

	// 跨蓝图粘贴可能与既有事件名、函数名冲突，冲突名由刷新会话追加序号
	RefreshEventFunctionName();
}

void UK2Node_SingularisSessionEvent::PostDuplicate(const bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	// 同蓝图内复制会与原节点同名，而同名 stub 会相互覆盖，故追加序号
	if (!bDuplicateForPIE)
		RefreshEventFunctionName();
}

void UK2Node_SingularisSessionEvent::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);

	// 1) 只关心订阅标签配置引脚
	if (!Pin || Pin->PinName != GetEventTagPinName())
		return;

	// 2) 引脚默认值按结构体导出文本解析（同时兼容裸标签名），写入节点侧单一事实
	FGameplayTag ConfiguredTag;
	ConfiguredTag.FromExportString(Pin->DefaultValue, PPF_SerializedAsImportText);
	if (SessionEventTag == ConfiguredTag)
		return;

	// 3) 标签变更即分派条件变更：事件函数名随之重推
	Modify();
	SessionEventTag = ConfiguredTag;
	RefreshEventFunctionName();
}

FText UK2Node_SingularisSessionEvent::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	// 配置未就绪时回退为通用标题
	if (!SessionEventTag.IsValid())
		return LOCTEXT("NodeTitle_None", "会话事件");

	return FText::Format(
		LOCTEXT("NodeTitle", "会话事件 \n {0}"),
		FText::FromString(SessionEventTag.ToString())
	);
}

FText UK2Node_SingularisSessionEvent::GetTooltipText() const
{
	return LOCTEXT(
		"NodeTooltip",
		"声明式会话事件处理器：无执行输入，直接从执行输出拉线写逻辑；负载输出引脚给出本次会话事件（可拆分取字段），在节点上选择订阅标签（限于事件域，层级匹配）即可，框架按标签直调本事件函数，无需任何额外入口"
	);
}

FText UK2Node_SingularisSessionEvent::GetKeywords() const
{
	// 检索词只影响菜单搜索命中，不参与任何语义
	return LOCTEXT("NodeKeywords", "会话事件 事件负载 声明式 分派 响应 交互 SessionEvent Interaction Event");
}

FLinearColor UK2Node_SingularisSessionEvent::GetNodeTitleColor() const { return FLinearColor(0.03f, 0.14f, 0.24f); }

FLinearColor UK2Node_SingularisSessionEvent::GetNodeTitleTextColor() const { return FLinearColor(0.45f, 0.85f, 1.0f); }

FSlateIcon UK2Node_SingularisSessionEvent::GetIconAndTint(FLinearColor& OutColor) const
{
	// 复用插件图标区分于引擎事件节点；槽位由 FSingularisInteractionEditorStyle 在模块启动时注册
	OutColor = FLinearColor::White;
	return FSlateIcon(
		FSingularisInteractionEditorStyle::GetStyleSetName(),
		FSingularisInteractionEditorStyle::GetSessionEventIconName()
	);
}

void UK2Node_SingularisSessionEvent::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	// 1) 宿主蓝图必须为 USingularisInteractionResponse 派生（分派目标是响应类）
	const UBlueprint* const Blueprint = GetBlueprint();
	if (!Blueprint || !Blueprint->ParentClass || !Blueprint->ParentClass->IsChildOf(
		USingularisInteractionResponse::StaticClass()
	))
	{
		MessageLog.Error(TEXT("@0 必须位于 USingularisInteractionResponse 派生蓝图的事件图内"), this);
		return;
	}

	// 2) 订阅标签必须配置且可解析：标签即分派条件，未配置的声明无法被分派
	if (!SessionEventTag.IsValid())
		MessageLog.Error(TEXT("@0 的事件标签未配置或无效"), this);

	// 3) 订阅标签必须落在事件域子树内：域外标签不属会话事件词汇，处理器永远不会被分派。
	//    标签选择器已按事件域过滤，此处兜底粘贴、导入与历史资产的旁路写入
	if (SessionEventTag.IsValid() && !SessionEventTag.MatchesTag(GetSubscribableEventRoot()))
	{
		MessageLog.Warning(
			*FText::Format(
				LOCTEXT(
					"EventTagOutsideDomain",
					"@0 的订阅标签 {0} 不在事件域（{1}）子树内，事件分派不会命中该处理器"
				),
				FText::FromString(SessionEventTag.ToString()),
				FText::FromString(GetSubscribableEventRoot().ToString())
			).ToString(),
			this
		);
	}

	// 4) 事件不得携带参数：参数会编译进 stub 函数，框架只能以无参方式直调
	if (!UserDefinedPins.IsEmpty())
		MessageLog.Error(TEXT("@0 不允许添加参数：处理器负载经负载输出引脚读取"), this);
}

bool UK2Node_SingularisSessionEvent::IsFunctionEntryCompatible(const UK2Node_FunctionEntry* EntryNode) const
{
	// 本节点的事件签名恒为空：处理器以无参方式被直调，负载与订阅标签引脚均不参与签名，
	// 故不能沿用引擎基类的逐引脚比对（基类会把负载引脚计为签名参数）；
	// 判据与基类同一口径：入口节点出现任何签名引脚，即与产物形态不一致。
	if (!EntryNode)
		return false;

	for (const UEdGraphPin* const Pin : EntryNode->Pins)
	{
		if (!Pin)
			continue;

		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec
			|| Pin->PinType.PinSubCategory == UEdGraphSchema_K2::PSC_Self
			|| Pin->PinName == DelegateOutputName
			|| Pin->Direction == EGPD_Input
			|| Pin->ParentPin != nullptr)
			continue;

		return false;
	}

	return true;
}

void UK2Node_SingularisSessionEvent::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// 1) 负载引脚未连线：无人消费本帧事件，无需接读取（拆分引脚的连线已由基类展开回本引脚）
	UEdGraphPin* const PayloadPin = FindPin(GetPayloadPinName());
	if (!PayloadPin || PayloadPin->LinkedTo.IsEmpty())
		return;

	// 2) 负载不是事件签名参数：展开期把连线改接分派帧读取，读取结果即本帧推送的会话事件
	UK2Node_CallFunction* const ReaderNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(
		this,
		SourceGraph
	);
	ReaderNode->FunctionReference.SetSelfMember(
		GET_FUNCTION_NAME_CHECKED(USingularisInteractionResponse, GetCurrentSessionEvent)
	);
	ReaderNode->AllocateDefaultPins();
	CompilerContext.MovePinLinksToIntermediate(*PayloadPin, *ReaderNode->GetReturnValuePin());
}

void UK2Node_SingularisSessionEvent::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	// 动作注册器未开放本节点类时直接返回，避免重复注册
	UClass* const ActionClass = GetClass();
	if (!ActionRegistrar.IsOpenForRegistration(ActionClass))
		return;

	UBlueprintNodeSpawner* const Spawner = UBlueprintNodeSpawner::Create(ActionClass);
	check(Spawner);
	ActionRegistrar.AddBlueprintAction(ActionClass, Spawner);
}

FText UK2Node_SingularisSessionEvent::GetMenuCategory() const { return LOCTEXT("MenuCategory", "引力奇点交互响应"); }

bool UK2Node_SingularisSessionEvent::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	// 基类已限定事件图（GT_Ubergraph）：事件节点只在 ubergraph 内生成可按名直调的 stub 函数
	if (!Super::IsCompatibleWithGraph(TargetGraph))
		return false;

	// 叠加宿主蓝图约束：分派目标必须是响应派生蓝图
	const UBlueprint* const Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);
	return Blueprint && Blueprint->ParentClass && Blueprint->ParentClass->IsChildOf(
		USingularisInteractionResponse::StaticClass()
	);
}

bool UK2Node_SingularisSessionEvent::IsConnectionDisallowed(
	const UEdGraphPin* MyPin,
	const UEdGraphPin* OtherPin,
	FString& OutReason
) const
{
	// 事件标签是配置项而非数据流：禁止连线，避免连线后配置静默失效
	if (MyPin && MyPin->PinName == GetEventTagPinName())
	{
		OutReason = LOCTEXT("EventTagPinNoConnection", "事件标签为配置项：请在节点上直接选择，不支持连线").ToString();
		return true;
	}

	return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);
}

FString UK2Node_SingularisSessionEvent::GetPinMetaData(const FName InPinName, const FName InKey)
{
	// 订阅标签引脚申报域限元数据：与项目内受域限标签属性同键约定，
	// 标签选择器据此把可选标签限制在事件域子树，并拒绝手填域外标签
	if (InPinName == GetEventTagPinName())
	{
		if (InKey == TagDomainMetadataKey)
			return GetSubscribableEventRoot().ToString();
		if (InKey == TagDomainForceSelectionKey)
			return TEXT("true");
	}

	return Super::GetPinMetaData(InPinName, InKey);
}

void UK2Node_SingularisSessionEvent::HideDelegateOutputPin() const
{
	// 委托输出引脚为编译器入口约定，本节点不暴露给蓝图作者
	if (UEdGraphPin* const DelegatePin = FindPin(DelegateOutputName))
		DelegatePin->bHidden = true;
}

void UK2Node_SingularisSessionEvent::RefreshEventFunctionName()
{
	// 1) 目标名：标签可解析时由标签推导，否则占位
	FString DesiredName = BuildFunctionNameFromTag(SessionEventTag).ToString();

	// 2) 类内去重：与既有事件名、函数图名、父类函数名冲突时追加序号
	if (const UBlueprint* const Blueprint = GetBlueprint())
	{
		TSet<FName> UsedNames;
		FBlueprintEditorUtils::GetFunctionNameList(Blueprint, UsedNames);

		TArray<UK2Node_Event*> EventNodes;
		FBlueprintEditorUtils::GetAllNodesOfClass<UK2Node_Event>(Blueprint, EventNodes);
		for (const UK2Node_Event* const EventNode : EventNodes)
		{
			if (EventNode && EventNode != this && !EventNode->CustomFunctionName.IsNone())
				UsedNames.Add(EventNode->CustomFunctionName);
		}

		FString UniqueName = DesiredName;
		for (auto Suffix = 1; UsedNames.Contains(FName(*UniqueName)); ++Suffix)
			UniqueName = FString::Printf(TEXT("%s_%d"), *DesiredName, Suffix);

		DesiredName = UniqueName;
	}

	// 3) 名称变化时写入（幂等：相同则不动，避免蓝图被误标脏）
	const FName NewFunctionName(*DesiredName);
	if (CustomFunctionName == NewFunctionName)
		return;

	Modify();
	CustomFunctionName = NewFunctionName;
}

#undef LOCTEXT_NAMESPACE
