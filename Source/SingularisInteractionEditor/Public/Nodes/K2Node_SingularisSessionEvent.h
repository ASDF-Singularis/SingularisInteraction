/* ====================================================================== *
 * K2Node_SingularisSessionEvent.h                                        *
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

#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <K2Node_Event.h>
#include <Textures/SlateIcon.h>

#include "K2Node_SingularisSessionEvent.generated.h"

/**
 * 会话事件节点（声明式分派）。
 *
 * 继承引擎事件基类，拖入交互响应蓝图的事件图即为真实入口：与引擎事件节点一致没有执行输入，
 * 从执行输出拉线写处理器逻辑，负载输出引脚按处理器入参同序给出本帧会话事件的会话上下文、量值与瞬态负荷，节点上选择订阅标签
 * （层级匹配，订阅父标签即覆盖全部后代）。
 * 编译器为本节点生成独立 stub UFunction（函数名由订阅标签推导，跳转 ubergraph），编译期由 hook
 * 把「标签 → 事件函数名」烘焙进响应类 CDO，运行时框架按名直调，无需任何额外入口。
 * 事件负载在分派帧内推送，负载引脚在节点展开期改写为分派帧读取（会话上下文、量值与瞬态负荷各一路），
 * 本节点不携带事件签名参数。
 *
 * 订阅标签的取值限于事件域（`Singularis.Interaction.Event`）子树，领域事件经其子标签开放扩展；
 * 新建节点默认订阅事件域达成，配置引脚不参与事件签名。
 */
UCLASS()
class UK2Node_SingularisSessionEvent : public UK2Node_Event
{
	GENERATED_BODY()

	/** 订阅标签（节点侧单一事实；配置引脚是其编辑入口，两者只在引脚变更时同步） */
	UPROPERTY()
	FGameplayTag SessionEventTag{};

public:
	/** 入场默认订阅事件域达成：新建节点即携带可用的订阅标签与事件函数名 */
	UK2Node_SingularisSessionEvent();

	/** 订阅标签配置引脚名（写入、读回、校验共用，避免魔法字符串漂移） */
	static FName GetEventTagPinName() { return FName(TEXT("EventTag")); }

	/** 会话上下文输出引脚名（本帧上下文快照；展开期改写为分派帧读取） */
	static FName GetContextPinName() { return FName(TEXT("Context")); }

	/** 量值输出引脚名（本帧量值；展开期改写为分派帧读取） */
	static FName GetValuePinName() { return FName(TEXT("Value")); }

	/** 瞬态负荷输出引脚名（本帧瞬态负荷；展开期改写为分派帧读取） */
	static FName GetPayloadPinName() { return FName(TEXT("Payload")); }

	/**
	 * 读取订阅标签。
	 *
	 * @return 订阅的事件标签；未配置时返回无效标签。
	 */
	const FGameplayTag& GetConfiguredEventTag() const { return SessionEventTag; }

	/**
	 * 由订阅标签推导事件函数名（合法标识符形态；类内冲突由刷新会话去重）。
	 *
	 * @param EventTag 订阅的事件标签。
	 * @return 事件函数名；标签无效时返回占位名。
	 */
	static FName BuildFunctionNameFromTag(const FGameplayTag& EventTag);

	/** 加载后隐藏基类委托输出引脚，使旧资产与新建节点形态一致 */
	virtual void PostLoad() override;

	/** 分配引脚：委托输出引脚隐藏，负载输出引脚按处理器入参同序给出会话上下文、量值与瞬态负荷，配置引脚镜像订阅标签 */
	virtual void AllocateDefaultPins() override;

	/** 新放置节点即固化事件函数名（stub 函数名必须非空） */
	virtual void PostPlacedNewNode() override;

	/** 粘贴后重推事件函数名，规避目标蓝图的既有名称冲突 */
	virtual void PostPasteNode() override;

	/** 非 PIE 复制后重推事件函数名，规避同蓝图内同名 stub 相互覆盖 */
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	/** 订阅标签引脚变更即回写节点侧订阅标签并重推事件函数名 */
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;

	/** 节点标题：会话事件 + 订阅标签，未配置时回退通用标题 */
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	/** 节点悬浮提示：声明式处理器形态与负载读取方式 */
	virtual FText GetTooltipText() const override;

	/** 菜单检索词 */
	virtual FText GetKeywords() const override;

	/** 标题栏着色 */
	virtual FLinearColor GetNodeTitleColor() const override;

	/** 标题栏文本着色 */
	virtual FLinearColor GetNodeTitleTextColor() const override;

	/** 节点图标：复用插件事件图标（注册于 FSingularisInteractionEditorStyle） */
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;

	/** 编译期校验：宿主蓝图、订阅标签的配置与事件域约束、无参约束 */
	virtual void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;

	/** 事件签名比对：本节点签名恒为空，负载与订阅标签引脚不计入签名 */
	virtual bool IsFunctionEntryCompatible(const UK2Node_FunctionEntry* EntryNode) const override;

	/** 节点展开：负载引脚改接分派帧读取，事件函数保持无参直调 */
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	/** 注册到蓝图右键菜单 */
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;

	/** 菜单分类路径（SingularisInteraction|Declarative） */
	virtual FText GetMenuCategory() const override;

	/** 图兼容性：仅允许放置于响应派生蓝图的事件图 */
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;

	/** 订阅标签为配置项而非数据流：禁止连线 */
	virtual bool IsConnectionDisallowed(
		const UEdGraphPin* MyPin,
		const UEdGraphPin* OtherPin,
		FString& OutReason
	) const override;

	/** 引脚元数据：为订阅标签引脚登记事件域过滤，限定标签选择器的可选范围 */
	virtual FString GetPinMetaData(FName InPinName, FName InKey) override;

private:
	/** 隐藏引擎事件基类暴露的委托输出引脚（框架按名直调本事件，蓝图侧无需绑定委托） */
	void HideDelegateOutputPin() const;

	/** 依据订阅标签刷新事件函数名（事件函数名即编译期生成的 stub 函数名，必须非空） */
	void RefreshEventFunctionName();
};
