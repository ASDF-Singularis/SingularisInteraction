/* ====================================================================== *
 * SingularisInteractionBlueprintCompileHooks.cpp                         *
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

#include "Nodes/SingularisInteractionBlueprintCompileHooks.h"

#include <EdGraph/EdGraph.h>
#include <Engine/Blueprint.h>
#include <UObject/Class.h>
#include <UObject/UObjectGlobals.h>

#include "SingularisInteractionEditor.h"
#include "Nodes/K2Node_SingularisSessionEvent.h"
#include "Objects/SingularisInteractionResponse.h"

void FSingularisInteractionBlueprintCompileHook::Register(FDelegateHandle& OutHandle)
{
	OutHandle = FCoreUObjectDelegates::OnObjectPostCDOCompiled.AddStatic(
		&FSingularisInteractionBlueprintCompileHook::HandleCDOCompiled
	);
}

void FSingularisInteractionBlueprintCompileHook::Unregister(FDelegateHandle& Handle)
{
	if (Handle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPostCDOCompiled.Remove(Handle);
		Handle.Reset();
	}
}

/**
 * CDO 编译完成回调：扫描事件图内的会话事件节点，把「标签 → 事件函数名」回填至响应类 CDO
 *
 * 仅收录事件图（GT_Ubergraph）内的声明：事件节点由编译器生成独立 stub UFunction，可按名直调。
 * 事件函数须无入参、无返回值（框架以无参方式直调）；同标签重复声明只保留首个。
 * 值未变化时跳过（避免编辑器加载误标脏）；变化时回填并标记蓝图待保存，使升级后的旧资产重新落盘。
 *
 * @param CDO 刚编译完成的类默认对象
 * @param Context 编译上下文（含骨架编译标志）
 */
void FSingularisInteractionBlueprintCompileHook::HandleCDOCompiled(
	// ReSharper disable once CppParameterMayBeConstPtrOrRef
	UObject* CDO,
	const FObjectPostCDOCompiledContext& Context
)
{
	// 1) 仅全量编译后的产物 CDO 参与回填，跳过骨架类编译
	if (Context.bIsSkeletonOnly)
		return;

	// 2) 仅处理 USingularisInteractionResponse 派生蓝图
	UClass* const GeneratedClass = CDO ? CDO->GetClass() : nullptr;
	if (!GeneratedClass || !GeneratedClass->IsChildOf(USingularisInteractionResponse::StaticClass()))
		return;

	const UBlueprint* const Blueprint = UBlueprint::GetBlueprintFromClass(GeneratedClass);
	if (!Blueprint)
		return;

	// 3) 收集事件图名：事件节点只在 ubergraph 页内生成可按名直调的 stub UFunction
	TSet<FName> EventGraphNames;
	for (const TObjectPtr<UEdGraph>& EventGraph : Blueprint->UbergraphPages)
	{
		if (EventGraph)
			EventGraphNames.Add(EventGraph->GetFName());
	}

	// 4) 扫描全部图中的声明节点：事件图内的声明入表，其余位置丢弃并告警
	TMap<FGameplayTag, FName> Backfilled;
	auto NodeCount = 0;
	TArray<UEdGraph*> AllGraphs;
	Blueprint->GetAllGraphs(AllGraphs);
	for (UEdGraph* Graph : AllGraphs)
	{
		if (!Graph)
			continue;

		TArray<UK2Node_SingularisSessionEvent*> Found;
		Graph->GetNodesOfClass<UK2Node_SingularisSessionEvent>(Found);

		for (const UK2Node_SingularisSessionEvent* Node : Found)
		{
			if (!Node)
				continue;

			++NodeCount;

			// 4.1) 非事件图：声明无法映射到可按名直调的事件函数，丢弃
			if (!EventGraphNames.Contains(Graph->GetFName()))
			{
				UE_LOG(
					LogSingularisInteractionEditor,
					Warning,
					TEXT("HandleCDOCompiled：忽略非事件图内的会话事件声明（蓝图=%s，图=%s）"),
					*GetNameSafe(Blueprint),
					*Graph->GetName()
				);
				continue;
			}

			// 4.2) 订阅标签未配置或无法解析
			const FGameplayTag EventTag = Node->GetConfiguredEventTag();
			if (!EventTag.IsValid())
			{
				UE_LOG(
					LogSingularisInteractionEditor,
					Warning,
					TEXT("HandleCDOCompiled：忽略事件标签未配置的会话事件声明（蓝图=%s）"),
					*GetNameSafe(Blueprint)
				);
				continue;
			}

			// 4.3) 事件函数必须存在且无入参、无返回值（框架以无参方式直调）
			const FName FunctionName = Node->CustomFunctionName;
			const UFunction* const HandlerFunction = FunctionName.IsNone()
				                                         ? nullptr
				                                         : GeneratedClass->FindFunctionByName(FunctionName);
			if (!HandlerFunction || HandlerFunction->NumParms != 0 || HandlerFunction->ReturnValueOffset != MAX_uint16)
			{
				UE_LOG(
					LogSingularisInteractionEditor,
					Warning,
					TEXT("HandleCDOCompiled：忽略事件函数缺失或带入参/返回值的声明（蓝图=%s，事件=%s）"),
					*GetNameSafe(Blueprint),
					*FunctionName.ToString()
				);
				continue;
			}

			// 4.4) 同标签重复声明只保留首个（声明表为标签 → 单处理器）
			FName& DeclaredFunction = Backfilled.FindOrAdd(EventTag);
			if (!DeclaredFunction.IsNone())
			{
				UE_LOG(
					LogSingularisInteractionEditor,
					Warning,
					TEXT("HandleCDOCompiled：事件标签重复声明，保留首个（标签=%s，事件=%s）"),
					*EventTag.ToString(),
					*FunctionName.ToString()
				);
				continue;
			}

			DeclaredFunction = FunctionName;
		}
	}

	UE_LOG(
		LogSingularisInteractionEditor,
		Display,
		TEXT("HandleCDOCompiled：扫描完成（蓝图=%s，节点=%d，声明=%d）"),
		*GetNameSafe(Blueprint),
		NodeCount,
		Backfilled.Num()
	);

	// 5) 回填响应类 CDO：值未变化时跳过（避免编辑器加载误标脏）；变化时回填并标记蓝图待保存
	USingularisInteractionResponse* const ResponseCDO = Cast<USingularisInteractionResponse>(CDO);
	if (!ResponseCDO || ResponseCDO->DeclaredSessionEventHandlers.OrderIndependentCompareEqual(Backfilled))
		return;

	ResponseCDO->DeclaredSessionEventHandlers = MoveTemp(Backfilled);
	// ReSharper disable once CppExpressionWithoutSideEffects
	ResponseCDO->MarkPackageDirty();

	UE_LOG(
		LogSingularisInteractionEditor,
		Display,
		TEXT("HandleCDOCompiled：回填声明表（CDO=%s，声明=%d）"),
		*ResponseCDO->GetPathName(),
		ResponseCDO->DeclaredSessionEventHandlers.Num()
	);
}
