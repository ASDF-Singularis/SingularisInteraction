# SingularisInteraction 架构设计文档 (Architecture Document)

> [!IMPORTANT]
> SingularisInteraction 是 Singularis 系列的标准参考实现，以系列标准模式**逻辑 (Logic) — 表现 (Presentation/UI) — 控制 (Control/Input)** 组织全部代码，在 Unreal Engine 5 中把交互流程拆解为**目标锁定 (Target Lock-on)**、**输入触发 (Input Trigger)**、**服务器权威执行 (Server-Authoritative Execution)** 与**表现驱动 (Presentation Driving)** 四个相互独立、可单独替换的扩展点。

## 概述 (Overview)

`SingularisInteraction`（引力奇点交互插件）是 Singularis 系列中面向交互领域的通用基础设施插件。插件不内建任何具体交互业务（拾取、开关、乘坐、开门等），只提供交互流程的骨架、数据契约与网络通路；具体副作用由项目侧以**交互策略 (Interaction Strategy)** 子类实现，表现层由项目侧以**视图 (View)** 实现。

在架构上，本插件是系列标准模式**逻辑 (Logic) — 表现 (Presentation/UI) — 控制 (Control/Input)** 的完整实例：交互组件与策略对象承担逻辑，两个控件组件与两个视图接口承担表现，交互者组件承担控制与输入（见 2.1）。

插件遵循系列的单体职责原则：运行时模块不依赖任何其他 Singularis 插件，删除其他插件不影响本插件的编译与运行。

| 项 | 值 |
| --- | --- |
| 插件名 / FriendlyName | `SingularisInteraction` |
| 版本 / 状态 | `0.1.0`，`IsBetaVersion = true`，`IsExperimentalVersion = false` |
| 资产分类 / 作者 | `Singularis` / TrifingZW |
| 许可 | MIT |
| 运行时模块 | `SingularisInteraction`（Type = `Runtime`，LoadingPhase = `Default`） |
| 编辑器模块 | `SingularisInteractionEditor`（Type = `Editor`，LoadingPhase = `Default`） |
| 插件级依赖 | `EnhancedInput` |
| Singularis 插件间依赖 | 无 |
| 可含内容 | 是（`CanContainContent = true`） |

运行时模块的模块级依赖（`SingularisInteraction.Build.cs`）：

| 分组 | 模块 |
| --- | --- |
| 核心 | `Core`、`CoreUObject`、`Engine`、`NetCore` |
| 表现 | `UMG`、`Slate`、`SlateCore` |
| 输入 | `InputCore`、`EnhancedInput` |
| 配置 | `EngineSettings`、`DeveloperSettings` |
| 语义 | `GameplayTags` |

编辑器模块的模块级依赖（`SingularisInteractionEditor.Build.cs`）：`Core`、`CoreUObject`、`Engine`、`Projects`、`SingularisInteraction`、`UMG`、`UMGEditor`、`UnrealEd`、`AssetTools`、`ContentBrowser`。

源码目录结构：

```
Plugins/SingularisInteraction/
├── SingularisInteraction.uplugin
├── Content/
│   ├── Inputs/                 # IA_Interaction、IMC_Default_SingularisInteractor
│   ├── Textures/               # T_* 图标资源
│   └── UserInterfaces/         # WBP_Default_SingularisInteractionWidget
└── Source/
    ├── SingularisInteraction/           # Runtime
    │   ├── Public/     # SingularisInteraction.h + Components / Configs / Interfaces / Objects / Subsystems / Types / Widgets
    │   └── Private/    # SingularisInteraction.cpp + 与 Public 同名的镜像目录
    └── SingularisInteractionEditor/     # Editor
        ├── Public/     # SingularisInteractionEditor.h + Factories
        └── Private/    # SingularisInteractionEditor.cpp + Factories
```

## 一、 设计目标与边界 (Design Goals & Boundaries)

### 1. 设计目标

| 目标 | 实现手段 |
| --- | --- |
| **三元分离 (Logic — Presentation — Control)**：三类职责分离且依赖单向 | 逻辑端不含输入与 UI 代码；表现端只经两个视图接口订阅状态与事件；控制端经网络边界向逻辑端提交请求（见 2.1） |
| **行为中性 (Behavior Neutrality)**：插件不规定“交互做什么”，只规定“交互如何发生” | 交互内容收口于 `USingularisInteractionStrategy` 抽象基类的 `Execute` |
| **数据驱动 (Data-Driven)**：策划在编辑器中完成装配，无需改动 C++ | `EditInlineNew` + `Instanced` 策略对象、`TMap<FGameplayTag, FSingularisInteractionStrategyPipeline>` 管线映射、`FComponentReference` 组件引用 |
| **零紧耦合 (Zero Tight Coupling)**：表现层不依赖具体控件类型 | 视图经 `ISingularisInteractionViewInterface` / `ISingularisInteractorViewInterface` 两个**接口契约 (Interface Contract)** 接入，任意 `UObject` 均可作为视图 |
| **服务器权威 (Server Authority)**：业务副作用只在服务器结算 | `TryInteraction` 标注 `BlueprintAuthorityOnly`，经由 `Server` `Reliable` `WithValidation` RPC 过桥 |
| **网络复制内建 (Built-in Replication)**：策略对象作为一等子对象参与复制与 RPC 路由 | 子对象复制列表（`AddReplicatedSubObject`）+ 策略基类覆写 `IsSupportedForNetworking` / `GetFunctionCallspace` / `CallRemoteFunction` |
| **蓝图完整暴露 (Blueprint Parity)**：C++ 能力全部可由蓝图驱动与扩展 | `BlueprintNativeEvent` SPI、`BlueprintCallable` / `BlueprintPure` API、中文 `DisplayName` 元数据 |

### 2. 边界（非目标）

- 不实现任何具体交互行为，不提供开箱即用的“拾取/开门/乘坐”逻辑。
- 不管理玩家状态、背包、任务等业务数据，不与任何 Singularis 业务插件通信。
- 不规定 UI 布局与表现：插件提供默认控件类与可选图标资源，具体布局由项目侧的视图实现决定。
- 不实现命中过滤的业务规则（如队伍、阵营、权限）；过滤应由项目侧覆写查询器或策略实现。
- 不绑定具体输入设备与按键，输入映射由 `UInputMappingContext` 与 `FSingularisInteractorInput` 数据配置。

## 二、 架构总览 (Architecture Overview)

### 1. 系列标准模式：逻辑 — 表现 — 控制 (Logic — Presentation — Control)

`Singularis` 系列以**逻辑 (Logic) — 表现 (Presentation/UI) — 控制 (Control/Input)** 三元结构作为标准模式：逻辑端承载领域状态与权威结算，表现端只读消费状态与事件，控制端采集输入并跨网络边界提交请求。本插件是该模式在交互域的实例，可作为新插件对齐的模板。

| 维度 | 职责 | 本插件载体 | 对外契约 |
| --- | --- | --- | --- |
| **逻辑 (Logic)** | 承载领域状态与权威结算，不感知输入设备与表现层 | `USingularisInteractionComponent`、`USingularisInteractionStrategy`、`USingularisInteractionBehaviorStrategy`、`USingularisInteractionSubsystem` | 状态访问器（`Enabled` / `Hovered`）、事件分发器、`TryInteraction`（`BlueprintAuthorityOnly`）、策略与行为策略 SPI |
| **表现 (Presentation/UI)** | 消费状态与事件并渲染；只读，不参与判定 | `ISingularisInteractionViewInterface`、`ISingularisInteractorViewInterface`、`USingularisInteractionWidgetComponent`、`USingularisInteractorWidgetComponent`、两个默认控件 | 两个视图接口：全量刷新（`OnRefresh`）+ 增量事件 |
| **控制 (Control/Input)** | 采集输入、锁定目标、跨网络边界提交请求 | `USingularisInteractorComponent`、`USingularisInteractionQueryer`、`FSingularisInteractorInput` | 增强输入绑定、`GetCurrentTarget`、事件分发器、`Server_RequestInteraction` RPC |

依赖方向与部署约定（本插件的实现方式）：

- **控制 → 逻辑：** 经网络边界单向提交（`Server_RequestInteraction` → `TryInteraction`）；悬浮状态由控制端在本地写入（`SetHovered`），该状态不参与逻辑结算。
- **逻辑 → 表现、控制 → 表现：** 均为“绑定后全量拉取 + 订阅增量事件”的单向推送；框架不向表现端提供回写通路（视图若需改变状态，须自行调用逻辑端或控制端的公开 API）。
- **表现端成对部署：** 目标侧表现（`USingularisInteractionWidgetComponent`）观察逻辑端组件；玩家侧表现（`USingularisInteractorWidgetComponent`）观察控制端组件。
- **逻辑不引用控制与表现：** 逻辑端组件与策略基类不含输入、控件与摄像机相关代码；控制端不依赖具体视图类型。

### 2. 分层结构

| 维度 | 层 | 类型 | 说明 |
| --- | --- | --- | --- |
| 共用 | 数据层 | `USTRUCT` / `UENUM` / `FGameplayTag` | 管线、条目、上下文、查询参数与结果、输入映射、原生标签 |
| 控制 / 逻辑 | 对象层 | `USingularisInteractionQueryer`（控制）、`USingularisInteractionStrategy` / `USingularisInteractionBehaviorStrategy`（逻辑） | 以 `Instanced` 内联子对象形式存在的行为单元，可被蓝图继承 |
| 逻辑 + 控制 | 组件层 | `USingularisInteractionComponent`（逻辑）、`USingularisInteractorComponent`（控制） | 交互语义的承载者与驱动者 |
| 逻辑 | 子系统层 | `USingularisInteractionSubsystem` | 世界级几何体 ↔ 交互组件映射表 |
| 表现 | 视图层 | 两个视图接口、两个控件组件、两个默认控件 | 状态与事件的消费端，仅依赖接口 |

### 3. 核心类职责总表

| 类 | 维度 | 基类 | 挂载点 | 职责 |
| --- | --- | --- | --- | --- |
| `USingularisInteractionComponent` | 逻辑 | `UActorComponent` | 可交互 Actor | 维护启用与悬浮状态；按标签层级分发交互策略管线；驱动行为策略；登记几何体映射与策略子对象 |
| `USingularisInteractorComponent` | 控制 | `UActorComponent` | `APlayerController` | 逐帧视线查询；维护当前锁定目标与悬浮副作用；动态切换输入映射上下文；将本地输入经 RPC 转交服务器 |
| `USingularisInteractionQueryer` | 控制 | `UObject` | 交互者组件的内联子对象 | 目标搜索算法；默认实现为单次 LineTrace |
| `USingularisInteractionStrategy` | 逻辑 | `UObject` | 交互组件的内联子对象 | 触发语义执行单元，按标签层级匹配后依次执行 |
| `USingularisInteractionBehaviorStrategy` | 逻辑 | `UObject` | 交互组件的内联子对象 | 状态语义执行单元，响应启用、禁用、悬浮、未悬浮 |
| `USingularisInteractionSubsystem` | 逻辑 | `UWorldSubsystem` | 世界 | 登记碰撞组件到交互组件的映射，供查询器命中的几何体反查交互语义 |
| `USingularisInteractionWidgetComponent` | 表现 | `UActorComponent` | 可交互 Actor | 目标侧观察者：实例化视图、装配提示范围重叠、订阅交互组件事件并推送 |
| `USingularisInteractorWidgetComponent` | 表现 | `UActorComponent` | `APlayerController` | 玩家侧观察者：实例化视图、订阅交互者组件事件并推送 |
| `USingularisInteractionWidget` / `USingularisInteractorWidget` | 表现 | `UUserWidget` | 视口 / 控件组件 | 默认视图实现，C++ 中为空实现，供蓝图或 C++ 子类覆写 |
| `USingularisInteractionSettings` | 共用 | `UDeveloperSettings` | 项目设置 | 插件级设置入口，聚合于 `Singularis` 分类 |

### 4. 运行结构图

```
[本地客户端 Local Client]
USingularisInteractorComponent  (挂在 APlayerController)
 ├─ TickComponent → Query()
 │    └─ USingularisInteractionQueryer.Query()
 │         LineTrace(ECC_GameTraceChannel1, TraceDistance)
 │              ▼
 │       FSingularisInteractionQueryerResult
 │         └─ SetQueryerResult() ─→ ApplyQueryerResult()
 │              ├─ 旧目标 SetHovered(false) / 新目标 SetHovered(true)
 │              ├─ RefreshInput()（添加 / 移除 UInputMappingContext）
 │              └─ OnInteractionTargetChangedEvent ─→ 交互者视图
 ├─ EnhancedInput Started → HandleInteractionAction()
 │         ├─ OnInteractionTriggeredEvent ─→ 交互者视图
 │         └─ Server_RequestInteraction (Server / Reliable / WithValidation)
 │                                                    │
[服务器 Server]                                       ▼
USingularisInteractionComponent.TryInteraction(StrategyTag, PlayerController, InputValue)
 ├─ OnInteractionEvent.Broadcast()  ─→ 交互控件组件 → 交互视图
 ├─ 组装 FSingularisInteractionStrategyContext
 └─ 按标签层级匹配 InteractionStrategyPipelineMapping
      └─ 逐条 Strategy.Execute(Context)   ← 项目侧副作用；可经子对象 RPC 回传表现
```

图中各参与者与标准模式三元的对应：`USingularisInteractionComponent` 与策略对象为逻辑端，`USingularisInteractorComponent`（含查询器与输入绑定）为控制端，两个控件组件为表现端。

### 5. 标签空间 (Tag Namespace)

原生标签在模块内以 `UE_DECLARE_GAMEPLAY_TAG_EXTERN` / `UE_DEFINE_GAMEPLAY_TAG_COMMENT` 声明（`Types/SingularisInteractionGameplayTags.h` / `.cpp`）：

| 标识符 | 标签 | 用途 |
| --- | --- | --- |
| `SingularisInteraction` | `Singularis.Interaction` | 交互域根标签 |
| `SingularisInteraction_Strategy` | `Singularis.Interaction.Strategy` | 策略域根标签 |
| `SingularisInteraction_Strategy_Default` | `Singularis.Interaction.Strategy.Default` | 默认策略标签，绑定默认输入动作 `IA_Interaction` |

`InteractionStrategyPipelineMapping` 与 `FSingularisInteractorInput::StrategyTag` 均带 `Categories = "Singularis.Interaction.Strategy"` 与 `ForceSelection = "true"` 元数据，编辑器侧限定在策略子树内取值。

## 三、 核心数据模型 (Data Model)

### 1. 交互策略管线 (Interaction Strategy Pipeline)

`FSingularisInteractionStrategyEntry`：单条策略条目。

| 字段 | 类型 | 语义 |
| --- | --- | --- |
| `StrategyName` | `FText` | 编辑器展示名，同时作为管线数组的 `TitleProperty` |
| `StrategyDescription` | `FText` | 策略说明 |
| `Strategy` | `USingularisInteractionStrategy*`（`Instanced`） | 策略实例，可在编辑器内联创建 |

`FSingularisInteractionStrategyPipeline`：一组有序策略。

| 字段 | 类型 | 语义 |
| --- | --- | --- |
| `Strategies` | `TArray<FSingularisInteractionStrategyEntry>` | 按数组顺序依次执行的策略集 |
| `bSuspend` | `bool`（默认 `true`） | 声明为“是否在策略完成后挂起后续策略”；当前执行路径未读取该字段（见第八章） |

`FSingularisInteractionStrategyEntry` 之外，交互组件另持有 `FSingularisInteractionBehaviorStrategyEntry`（字段：`BehaviorStrategyName`、`BehaviorStrategyDescription`、`BehaviorStrategy`），结构与策略条目一致，用于状态语义而非触发语义。

### 2. 交互策略上下文 (Strategy Context)

`FSingularisInteractionStrategyContext`：由 `USingularisInteractionComponent::TryInteraction` 组装，描述一次触发中的身份与输入。

| 字段 | 类型 | 来源 |
| --- | --- | --- |
| `Controller` | `AController*` | RPC 传入的 `APlayerController` |
| `Instigator` | `APawn*` | `Controller->GetPawn()` |
| `Avatar` | `AActor*` | 交互组件宿主 `GetOwner()` |
| `Target` | `TObjectPtr<AActor>` | 当前实现与 `Avatar` 相同，均为 `GetOwner()` |
| `InteractionComponent` | `USingularisInteractionComponent*` | 触发本次交互的交互组件自身 |
| `InputValue` | `FInputActionValue` | 触发交互的增强输入值 |

### 3. 交互行为策略上下文 (Behavior Strategy Context)

`FSingularisInteractionBehaviorStrategyContext`：由 `SetEnabled` / `SetHovered` 组装，描述副作用作用的交互目标。

| 字段 | 类型 | 来源 |
| --- | --- | --- |
| `InteractionActor` | `AActor*` | 交互组件宿主 `GetOwner()` |
| `InteractionComponent` | `USingularisInteractionComponent*` | 交互组件自身 |

### 4. 查询参数与结果 (Queryer Params & Result)

`FSingularisInteractionQueryerParams`：

| 字段 | 类型 | 语义 |
| --- | --- | --- |
| `ViewLoc` | `FVector` | 视线起点（世界坐标），由 `APlayerController::GetPlayerViewPoint` 提供 |
| `ViewRot` | `FRotator` | 视线朝向 |
| `IgnoredActor` | `AActor*` | 需忽略的 Actor，当前实现传入交互者的 Pawn |

`FSingularisInteractionQueryerResult`：

| 字段 | 类型 | 语义 |
| --- | --- | --- |
| `InteractionActor` | `AActor*` | 命中的 Actor |
| `InteractionComponent` | `USingularisInteractionComponent*` | 命中几何体上登记的交互组件 |
| `ImpactPoint` | `FVector_NetQuantize` | 命中点（世界坐标） |

两个成员函数：

- `IsInteractionValid()`：`InteractionActor` 与 `InteractionComponent` 均有效时返回 `true`。
- `operator==`：仅比较 `InteractionActor` 与 `InteractionComponent`，不比较 `ImpactPoint`；该比较是交互者组件状态幂等判定的依据——同一目标内的命中点变化不产生状态更新与事件广播。

### 5. 交互者输入映射 (Interactor Input)

`FSingularisInteractorInput`：把增强输入动作映射到交互策略标签。

| 字段 | 类型 | 语义 |
| --- | --- | --- |
| `InputAction` | `UInputAction*` | 输入动作 |
| `StrategyTag` | `FGameplayTag` | 该动作触发时作为 `TryInteraction` 的策略标签 |

### 6. 交互模式枚举 (Interaction Mode)

`ESingularisInteractionMode`：`Ray`（射线查询，显示名“射线”）、`Collision`（碰撞查询，显示名“碰撞”）。该枚举当前由查询器的 `SingularisInteractionMode` 属性声明配置入口，但基类实现未读取（见第八章）。

## 四、 核心模块解析 (Core Module Breakdown)

### 1. `USingularisInteractionComponent`（交互组件，目标侧）

元数据：`UCLASS(Blueprintable, BlueprintType, ClassGroup = ("Singularis"), meta = (BlueprintSpawnableComponent, DisplayName = "引力奇点交互组件"))`，继承 `UActorComponent`。

构造期设定：`SetIsReplicatedByDefault(true)`、`bReplicateUsingRegisteredSubObjectList = true`、关闭 Tick（`bCanEverTick = false`、`bStartWithTickEnabled = false`）、`bAutoActivate = true`。

**配置参数**

| 参数 | 类型 | 语义 |
| --- | --- | --- |
| `TargetComponentReferences` | `TArray<FComponentReference>` | 交互目标组件引用（`UseComponentPicker`，`AllowedClasses = /Script/Engine.SceneComponent`），用于在映射子系统登记归属；允许多个几何体指向同一交互组件 |
| `InteractionStrategyPipelineMapping` | `TMap<FGameplayTag, FSingularisInteractionStrategyPipeline>` | 策略标签到管线的映射，触发时按标签层级筛选 |
| `InteractionBehaviorStrategies` | `TArray<FSingularisInteractionBehaviorStrategyEntry>` | 行为策略集，响应启用与悬浮状态变化 |

**状态与事件**

| 成员 | 说明 |
| --- | --- |
| `bIsEnabled`（默认 `true`） | 启用状态；私有普通成员，无 `UPROPERTY`，不参与复制 |
| `bIsHovered`（默认 `false`） | 悬浮状态；同上 |
| `OnInteractionEvent` | 开始交互时广播 |
| `OnInteractionEnableEvent` / `OnInteractionDisableEvent` | 启用 / 禁用时广播 |
| `OnInteractionHoverEvent` / `OnInteractionUnhoverEvent` | 悬浮 / 未悬浮时广播 |

**函数**

| 函数 | 说明 |
| --- | --- |
| `Enabled()` / `Hovered()` | `BlueprintPure` 状态访问器 |
| `SetEnabled(bool)` / `SetHovered(bool)` | `BlueprintCallable`；先做幂等比较（状态未变化直接返回），随后广播对应事件，并逐条驱动行为策略的 `Enabled` / `Disabled` 或 `Hovered` / `Unhovered` |
| `TryInteraction(StrategyTag, PlayerController, InputActionValue)` | `BlueprintAuthorityOnly`；广播 `OnInteractionEvent`，组装策略上下文，遍历映射并对满足 `Tag.MatchesTag(StrategyTag)` 的管线逐条调用 `Strategy->Execute(Context)` |
| `BeginPlay()` | 依次执行：登记策略子对象到复制列表 → 将 `TargetComponentReferences` 指向的 `UPrimitiveComponent` 登记进映射子系统 |
| `RegisterInteractionSubObjects()` | 内部函数，仅在 `HasAuthority()` 时向子对象复制列表登记全部交互策略与行为策略实例 |

**标签匹配语义**：`Tag.MatchesTag(StrategyTag)` 中的 `Tag` 为映射键。`A.MatchesTag(B)` 在 `A` 等于 `B` 或 `A` 是 `B` 的后代时为真，因此映射键是“被触发标签的等值或后代”。触发 `Singularis.Interaction.Strategy` 会执行映射中所有以该前缀登记的管线；触发 `Singularis.Interaction.Strategy.Default` 只执行 `Default` 及其子级登记项。映射遍历顺序为 `TMap` 迭代顺序，同一次触发可命中多个管线。

**行为策略的驱动端**：`SetEnabled` / `SetHovered` 在调用端就地执行行为策略副作用，不经过 RPC；跨端一致性由调用方决定。当前默认路径中，悬浮状态由本地交互者组件驱动（见 5.2），即悬浮副作用在本地客户端结算。

### 2. `USingularisInteractorComponent`（交互者组件，玩家侧）

元数据：`UCLASS(Blueprintable, BlueprintType, ClassGroup = ("Singularis"), meta = (BlueprintSpawnableComponent, DisplayName = "引力奇点交互者组件"))`，继承 `UActorComponent`。

构造期设定：`SetIsReplicatedByDefault(true)`、开启 Tick、`bAutoActivate = true`；创建默认查询器子对象 `"InteractionQueryer"`；通过 `ConstructorHelpers` 解析默认资产 `IMC_Default_SingularisInteractor` 与 `IA_Interaction`，并将 `IA_Interaction` 与 `Singularis.Interaction.Strategy.Default` 组成默认输入项写入 `InteractorInputs`。

`BeginPlay` 中执行 `checkf(GetOwner()->IsA<APlayerController>(), ...)`，即该组件必须挂在 `APlayerController` 上；随后缓存宿主控制器、绑定输入、刷新输入映射上下文。

**配置参数**

| 参数 | 类型 | 默认 | 语义 |
| --- | --- | --- | --- |
| `InteractionQueryer` | `USingularisInteractionQueryer*`（`Instanced`） | 构造期创建 | 目标搜索算法 |
| `InputPriority` | `int32` | `10` | 添加映射上下文时的优先级 |
| `InputMappingContext` | `UInputMappingContext*` | 默认资产 | 仅在锁定到有效目标期间挂载 |
| `InteractorInputs` | `TArray<FSingularisInteractorInput>` | 默认 1 项 | 输入动作到策略标签的映射表 |

**状态与事件**

| 成员 | 说明 |
| --- | --- |
| `OwnerPlayerController` | `TWeakObjectPtr<APlayerController>`，宿主的弱引用 |
| `CurrentQueryerResult` | 当前视线查询结果，纯本地状态，不参与复制 |
| `OnInteractionTargetChangedEvent(OldTarget, NewTarget)` | 目标变更时广播 |
| `OnInteractionTriggeredEvent(Target, StrategyTag)` | 发起交互请求时广播 |

**函数**

| 函数 | 说明 |
| --- | --- |
| `TickComponent` → `Query()` | 每帧执行；先校验本地控制器与查询器有效性，再取 `GetPlayerViewPoint` 构建查询参数（忽略目标为宿主 Pawn），执行查询并写入结果 |
| `SetQueryerResult(Result)` | 幂等比较（`operator==`）后写入新结果，捕获旧结果并调用 `ApplyQueryerResult`，最后广播目标变更事件 |
| `ApplyQueryerResult(OldResult)` | 旧目标有效则 `SetHovered(false)`；新目标有效则 `SetHovered(true)`；随后 `RefreshInput()` |
| `RefreshInput()` | 有有效目标时 `AddMappingContext(InputMappingContext, InputPriority)`，否则 `RemoveMappingContext` |
| `BindInput()` | 仅在本地控制器上，将 `InteractorInputs` 逐条以 `ETriggerEvent::Started` 绑定到 `UEnhancedInputComponent`，绑定函数为 `HandleInteractionAction`，`StrategyTag` 作为载荷传入 |
| `HandleInteractionAction(ActionValue, StrategyTag)` | 校验本地环境与当前目标后，广播 `OnInteractionTriggeredEvent`，并调用 `Server_RequestInteraction` |
| `Server_RequestInteraction` | `Server` `Reliable` `WithValidation` RPC；校验阶段目前仅检查目标组件有效性（源码中注明后续可补充距离校验）；实现阶段二次校验目标与宿主控制器，随后调用 `TargetInteractionComponent->TryInteraction(...)` |
| `GetCurrentTarget()` | `BlueprintPure`，返回 `CurrentQueryerResult.InteractionComponent` |

**关键约束**：查询、状态写入、输入绑定、RPC 发送全部受 `IsLocalController()` 约束，因此每个客户端只运行自己的交互者逻辑，专用服务器不执行查询。

### 3. `USingularisInteractionQueryer`（交互查询器）

元数据：`UCLASS(Blueprintable, BlueprintType, EditInlineNew, CollapseCategories)`，继承 `UObject`。

| 参数 | 类型 | 默认 | 语义 |
| --- | --- | --- | --- |
| `TraceDistance` | `float` | `200.0f`（厘米） | 射线距离 |
| `SingularisInteractionMode` | `ESingularisInteractionMode` | `Ray` | 查询模式（当前基类实现未读取） |

SPI：`Query(FSingularisInteractionQueryerResult& QueryerResult, const FSingularisInteractionQueryerParams& QueryerParams)`，`BlueprintNativeEvent`，返回是否命中有效可交互目标。

基类实现流程：

1. 由 `ViewLoc` 与 `ViewRot.Vector() * TraceDistance` 计算射线起止点。
2. 构建 `FCollisionQueryParams`（`SCENE_QUERY_STAT(LineTrace)`，`bTraceComplex = true`）并忽略 `IgnoredActor`。
3. 以 `ECC_INTERACTION`（源码内 `#define ECC_INTERACTION ECC_GameTraceChannel1`）执行 `LineTraceSingleByChannel`。
4. 命中后取命中组件的宿主 Actor，经 `USingularisInteractionSubsystem::MappingComponent` 反查交互组件；任一环节无效则返回 `false`。
5. 装配 `InteractionActor`、`InteractionComponent`、`ImpactPoint` 并返回 `true`。

私有静态函数 `FindInteractionComponent(Actor, PrimitiveComponent)` 封装上述反查，要求世界上下文中存在交互子系统。

### 4. `USingularisInteractionSubsystem`（交互子系统）

元数据：`UCLASS(NotBlueprintable, BlueprintType, ClassGroup = ("Singularis"), meta = (BlueprintSpawnableComponent, DisplayName = "引力奇点交互子系统"))`，继承 `UWorldSubsystem`。

数据成员为 `TMap<TWeakObjectPtr<UPrimitiveComponent>, TWeakObjectPtr<USingularisInteractionComponent>>`，键与值均为弱引用，对象销毁后自动失效，无需显式清理。

| 函数 | 语义 |
| --- | --- |
| `RegisterMapping(PrimitiveComponent, InteractionComponent)` | 登记映射；重复登记同一碰撞组件时覆盖既有映射 |
| `UnregisterMapping(PrimitiveComponent)` | 注销映射；未登记时幂等返回 |
| `MappingComponent(PrimitiveComponent)` | 哈希查找 → 弱引用有效性检查 → 解引用；未登记或失效返回 `nullptr` |

`Initialize` / `Deinitialize` 目前仅调用 `Super`。映射登记由交互组件的 `BeginPlay` 触发，仅在组件生命周期内执行一次；运行期新增或更换的几何体需由项目侧手动调用 `RegisterMapping`。

### 5. `USingularisInteractionStrategy`（交互策略，触发语义）

元数据：`UCLASS(Abstract, Blueprintable, EditInlineNew, CollapseCategories)`，继承 `UObject`。

SPI（均为 `BlueprintNativeEvent`）：

| 接口 | 默认实现 | 语义 |
| --- | --- | --- |
| `CanExecute(const FSingularisInteractionStrategyContext&) const` | 返回 `true` | 判定执行前提；由子类覆写 |
| `Execute(const FSingularisInteractionStrategyContext&)` | 空实现 | 执行交互副作用；由子类覆写 |

网络能力（为子对象 RPC 而覆写）：

| 覆写 | 实现 |
| --- | --- |
| `GetWorld()` | 排除 CDO 后沿 `GetOuter()` 链取世界上下文 |
| `IsSupportedForNetworking()` | 返回 `true` |
| `GetFunctionCallspace(Function, Stack)` | 排除 CDO 后委托给 `GetOuter()`（组件 → 宿主 Actor） |
| `CallRemoteFunction(...)` | 沿 Outer 链定位宿主 Actor 的 `UNetDriver`，调用 `ProcessRemoteFunction(Actor, Function, Parms, OutParms, Stack, this)`，将策略子对象自身作为路由对象传入 |
| `GetLifetimeReplicatedProps(...)` | 仅调用 `Super`，供子类追加复制属性 |

该组覆写使策略子类可以自行声明 `Server` / `Client` / `NetMulticast` RPC，并由宿主 Actor 的网络通道完成路由。

### 6. `USingularisInteractionBehaviorStrategy`（交互行为策略，状态语义）

元数据：`UCLASS(Abstract, Blueprintable, EditInlineNew, CollapseCategories)`，继承 `UObject`。网络能力覆写与交互策略基类一致（`GetWorld` / `IsSupportedForNetworking` / `GetFunctionCallspace` / `CallRemoteFunction` / `GetLifetimeReplicatedProps`）。

SPI（均为 `BlueprintNativeEvent`，默认空实现）：

| 接口 | 触发时机 |
| --- | --- |
| `Enabled(Context)` | 交互组件启用时 |
| `Disabled(Context)` | 交互组件禁用时 |
| `Hovered(Context)` | 交互组件被悬浮时 |
| `Unhovered(Context)` | 交互组件解除悬浮时 |

行为策略不参与交互触发管线，只响应状态变化。

### 7. 视图层 (View Layer)

#### 7.1 视图接口 (View Interfaces)

| 接口 | 方法 |
| --- | --- |
| `ISingularisInteractionViewInterface` | `OnRefresh(bEnabled, bHovered)`、`OnTrigger()`、`OnHover()`、`OnUnhover()`、`OnEnterRange()`、`OnExitRange()` |
| `ISingularisInteractorViewInterface` | `OnRefresh(Target)`、`OnTargetChanged(OldTarget, NewTarget)`、`OnTriggered(Target, StrategyTag)` |

两个接口均为 `UINTERFACE(Blueprintable, BlueprintType)`，全部方法为 `BlueprintNativeEvent` + `BlueprintCallable`。`OnRefresh` 是**全量刷新 (Full Pull)** 入口，由控件组件在绑定完成后主动调用，用于消除绑定前错过事件导致的空白期；其余方法为**增量事件 (Delta Event)**。

#### 7.2 控件组件 (Widget Components)

| 组件 | 挂载点 | 复制 | 关键配置 | 依赖方向 |
| --- | --- | --- | --- | --- |
| `USingularisInteractionWidgetComponent` | 可交互 Actor | `SetIsReplicatedByDefault(false)` | `WidgetComponentReference`（UI 承载）、`PromptVolumeReference`（`UShapeComponent` 提示范围）、`InteractionComponentReference`、`InteractionWidgetClass`（`MustImplement` 视图接口） | 订阅 `USingularisInteractionComponent` 事件 |
| `USingularisInteractorWidgetComponent` | `APlayerController` | `SetIsReplicatedByDefault(false)` | `bAutoCreateView`（默认 `true`）、`InteractorWidgetClass`（`MustImplement` 交互者视图接口，`EditCondition = bAutoCreateView`） | 订阅 `USingularisInteractorComponent` 事件 |

两者的视图引用均以 `TScriptInterface` 缓存（`Transient`），调用一律通过 `Execute_` 静态派发，因此视图可以是任意实现对应接口的 `UObject`，不限于 `UUserWidget`。

`USingularisInteractionWidgetComponent` 的 `BeginPlay` 流程：

1. `ProxyWidgetComponent()`：本地控制器检查 → 解析控件组件引用 → `CreateWidget` → 运行时以 `ensureMsgf` 复核控件类是否实现视图接口（`MustImplement` 仅约束编辑器选择器）→ 以 `EWidgetSpace::Screen` 与 `ECollisionEnabled::NoCollision` 装配。
2. `ProxyPromptVolume()`：将提示范围的 `ECC_INTERACTION` 响应设为 `Ignore`（避免提示范围遮挡视线查询），并绑定 `OnComponentBeginOverlap` / `OnComponentEndOverlap`；重叠回调仅在对方为 Pawn、其控制器为本地控制器且挂有交互者组件时，向视图推送 `OnEnterRange` / `OnExitRange`。
3. `ObserveInteractionComponent()`：订阅交互组件的开始交互、悬浮、未悬浮事件，随后以 `Enabled()` 与 `Hovered()` 推送一次 `OnRefresh`。

`USingularisInteractorWidgetComponent` 的 `BeginPlay` 中同样 `checkf` 要求宿主为 `APlayerController`；`bAutoCreateView = true` 时创建控件并添加到视口，随后订阅目标变更与交互触发事件并推送一次 `OnRefresh`。`bAutoCreateView = false` 时由 `SetInteractorView` 接受外部注入（自动创建模式下该函数直接返回），注入后自动推送一次全量状态。

#### 7.3 默认控件 (Default Widgets)

`USingularisInteractionWidget` 与 `USingularisInteractorWidget` 均继承 `UUserWidget` 并实现对应视图接口，C++ 中全部 SPI 为空实现，作为项目侧蓝图子类的挂载基类。交互控件组件的构造函数通过 `FClassFinder` 解析默认资产 `WBP_Default_SingularisInteractionWidget`。

### 8. 设置与编辑器集成 (Settings & Editor Integration)

`USingularisInteractionSettings`：`UCLASS(Config = SingularisInteraction, DefaultConfig)`，继承 `UDeveloperSettings`。编辑器分类名 `Singularis`，段落标题 `Singularis Interaction`，描述“引力奇点交互插件设置”；当前不包含配置字段。

`FSingularisInteractionEditorModule`：在 `StartupModule` 中于 `AssetTools` 注册资产分类 `Singularis`，随后注册五个资产行为；`ShutdownModule` 中在模块仍加载的前提下逐一注销并清空缓存数组。

| 资产行为 | 目标类 | 工厂产出 | 颜色 | 子菜单 |
| --- | --- | --- | --- | --- |
| `FAssetTypeActions_SingularisInteractionQueryer` | `USingularisInteractionQueryer` | `UBlueprint`（`BPTYPE_Normal`） | `FColor(0, 255, 0)` | `SingularisInteraction` |
| `FAssetTypeActions_SingularisInteractionStrategy` | `USingularisInteractionStrategy` | `UBlueprint` | `FColor(63, 126, 255)` | `SingularisInteraction` |
| `FAssetTypeActions_SingularisInteractionBehaviorStrategy` | `USingularisInteractionBehaviorStrategy` | `UBlueprint` | `FColor(63, 126, 255)` | `SingularisInteraction` |
| `FAssetTypeActions_SingularisInteractionWidget` | `USingularisInteractionWidget` | `UWidgetBlueprint` | `FColor(44, 89, 180)` | `SingularisInteraction` |
| `FAssetTypeActions_SingularisInteractorWidget` | `USingularisInteractorWidget` | `UWidgetBlueprint` | `FColor(44, 89, 180)` | `SingularisInteraction` |

五个资产行为均派生自 `FAssetTypeActions_Blueprint`，对应工厂（`UFactory` 子类）设置 `bCreateNew = true`、`bEditAfterNew = true`，经 `FKismetEditorUtilities::CreateBlueprint` 创建蓝图资产（控件类产出 `UWidgetBlueprint`）。

## 五、 运行机制与数据流 (Runtime Flow)

### 1. 装配阶段 (BeginPlay)

| 端 | 对象 | 动作 |
| --- | --- | --- |
| 所有端 | `USingularisInteractionComponent` | 服务器登记策略与行为策略子对象到复制列表；向世界子系统登记 `TargetComponentReferences` 映射 |
| 所有端 | `USingularisInteractorComponent` | 校验宿主为 `APlayerController`；本地控制器绑定输入并刷新映射上下文 |
| 本地客户端 | `USingularisInteractionWidgetComponent` | 创建交互视图、装配提示范围、订阅交互组件并推送全量状态 |
| 本地客户端 | `USingularisInteractorWidgetComponent` | 创建交互者视图（或等待外部注入）、订阅交互者组件并推送全量状态 |

### 2. 目标锁定循环 (Query Loop)

```
TickComponent → Query()
  ├─ 非本地控制器 / 查询器无效 → 返回
  ├─ GetPlayerViewPoint → ViewLoc / ViewRot
  ├─ Query(Params) → Result
  └─ SetQueryerResult(Result)
        ├─ 结果与当前状态等价（同一 Actor + 同一组件）→ 返回
        ├─ 旧目标 SetHovered(false)（如有效）
        ├─ 新目标 SetHovered(true)（如有效）
        ├─ RefreshInput()（挂载 / 卸载输入映射上下文）
        └─ OnInteractionTargetChangedEvent.Broadcast(Old, New)
```

悬浮是**状态迁移 (State Transition)** 而非每帧写入：只有目标发生变更时才调用 `SetHovered`，因此启用 / 禁用、悬浮 / 未悬浮的事件与行为策略副作用均为边沿触发。

### 3. 输入与网络通路 (Input & RPC Path)

```
EnhancedInput (Started)
  → USingularisInteractorComponent::HandleInteractionAction(ActionValue, StrategyTag)
      ├─ 本地控制器 + 有效目标校验
      ├─ OnInteractionTriggeredEvent.Broadcast(Target, StrategyTag)   [本地]
      └─ Server_RequestInteraction(Target, StrategyTag, ActionValue)  [Server / Reliable / WithValidation]
            → USingularisInteractionComponent::TryInteraction(StrategyTag, PlayerController, ActionValue)  [服务器]
```

输入映射上下文只在锁定到有效目标期间存在，因此“有目标才有交互按键”的准入规则由输入系统承担，插件不额外实现门控。

### 4. 权威执行与策略分发 (Authority & Strategy Dispatch)

`TryInteraction` 是唯一的交互结算入口，标注 `BlueprintAuthorityOnly`，只能由服务器调用。执行顺序：

1. 校验 `PlayerController` 有效。
2. 广播 `OnInteractionEvent`。
3. 组装 `FSingularisInteractionStrategyContext`（`Controller` / `Instigator` / `Avatar` / `Target` / `InteractionComponent` / `InputValue`）。
4. 遍历 `InteractionStrategyPipelineMapping`，对满足 `Tag.MatchesTag(StrategyTag)` 的每个管线的 `Strategies` 数组按序调用 `Strategy->Execute(Context)`。

当前实现不在管线层调用 `CanExecute`，也不读取 `bSuspend`；执行前提判定若需要，须由策略自身在 `Execute` 内部完成（见第八章）。

### 5. 状态驱动与视图事件 (State & View Events)

| 事件源 | 端 | 视图方法 | 视图类型 |
| --- | --- | --- | --- |
| `OnInteractionEvent`（`TryInteraction` 内广播） | 服务器 | `OnTrigger()` | 交互视图 |
| `OnInteractionHoverEvent` / `OnInteractionUnhoverEvent` | 调用 `SetHovered` 的端 | `OnHover()` / `OnUnhover()` | 交互视图 |
| 视图绑定时全量拉取 | 本地客户端 | `OnRefresh(bEnabled, bHovered)` | 交互视图 |
| 提示范围重叠 | 本地客户端 | `OnEnterRange()` / `OnExitRange()` | 交互视图 |
| `OnInteractionTargetChangedEvent` | 本地客户端 | `OnTargetChanged(Old, New)` | 交互者视图 |
| `OnInteractionTriggeredEvent` | 本地客户端 | `OnTriggered(Target, StrategyTag)` | 交互者视图 |
| 视图绑定时全量拉取 | 本地客户端 | `OnRefresh(Target)` | 交互者视图 |

### 6. 网络模型 (Network Model)

| 数据 / 动作 | 端 | 机制 |
| --- | --- | --- |
| 交互请求 `Server_RequestInteraction` | 客户端 → 服务器 | `Server` `Reliable` `WithValidation` |
| 策略执行 `TryInteraction` | 服务器 | `BlueprintAuthorityOnly` |
| 策略与行为策略实例 | 服务器 → 客户端 | `Instanced` 子对象 + `AddReplicatedSubObject`（仅 `HasAuthority` 时登记） |
| 交互者当前目标 `CurrentQueryerResult` | 本地客户端 | 普通成员，无 `UPROPERTY`，不复制；各端独立计算结果 |
| 交互组件启用 / 悬浮状态 `bIsEnabled` / `bIsHovered` | 本地 | 私有普通成员，不复制 |
| 交互组件与交互者组件事件委托 | 调用端本地 | `BlueprintAssignable` 动态多播委托，未标注 `Replicated` |
| 交互控件组件与交互者控件组件 | 本地客户端 | `SetIsReplicatedByDefault(false)` |
| 交互者组件 | 服务器 ↔ 客户端 | `SetIsReplicatedByDefault(true)`，用于承载 Server RPC |

### 7. 装配清单 (Integration Checklist)

| 位置 | 必需 | 组件 / 配置 |
| --- | --- | --- |
| 可交互 Actor | 必需 | `USingularisInteractionComponent`，并配置 `TargetComponentReferences` 指向参与命中的几何体 |
| 可交互 Actor | 可选 | `USingularisInteractionWidgetComponent` + `UWidgetComponent` + `UShapeComponent`（提示范围） |
| 玩家控制器 | 必需 | `USingularisInteractorComponent`（含查询器与输入映射配置） |
| 玩家控制器 | 可选 | `USingularisInteractorWidgetComponent` |
| 项目设置 | 必需 | 交互通道：源码使用 `ECC_GameTraceChannel1`，项目侧需将其命名为交互通道并让可交互几何体对该通道产生 `Block` 响应 |
| 项目设置 | 可选 | `Project Settings → Singularis → Singularis Interaction`（当前无配置项） |

本仓库 `Config/DefaultEngine.ini` 已将该通道登记为 `ECC_INTERACTION`，默认响应为 `ECR_Block`。

## 六、 扩展点清单 (Extension Points)

| 扩展点 | 声明位置 | 调用方 | 语义 |
| --- | --- | --- | --- |
| `USingularisInteractionQueryer::Query` | `Objects/SingularisInteractionQueryer.h` | `USingularisInteractorComponent::Query` | 替换目标搜索算法（球形检测、形状扫描、多重命中、多目标评分等） |
| `USingularisInteractionStrategy::CanExecute` | `Objects/SingularisInteractionStrategy.h` | 当前无调用方 | 执行前提判定；当前需由子类在 `Execute` 内自行调用或替代 |
| `USingularisInteractionStrategy::Execute` | `Objects/SingularisInteractionStrategy.h` | `USingularisInteractionComponent::TryInteraction` | 交互副作用（服务器权威） |
| `USingularisInteractionBehaviorStrategy::Enabled` / `Disabled` | `Objects/SingularisInteractionBehaviorStrategy.h` | `USingularisInteractionComponent::SetEnabled` | 启用状态副作用 |
| `USingularisInteractionBehaviorStrategy::Hovered` / `Unhovered` | `Objects/SingularisInteractionBehaviorStrategy.h` | `USingularisInteractionComponent::SetHovered` | 悬浮状态副作用 |
| `ISingularisInteractionViewInterface` | `Interfaces/SingularisInteractionViewInterface.h` | `USingularisInteractionWidgetComponent` | 目标侧表现（提示、图标、范围反馈、触发反馈） |
| `ISingularisInteractorViewInterface` | `Interfaces/SingularisInteractorViewInterface.h` | `USingularisInteractorWidgetComponent` | 玩家侧表现（准星、当前目标信息、输入反馈） |
| `USingularisInteractorComponent::SetInteractorView` | `Components/SingularisInteractorWidgetComponent.h` | 项目侧调用 | 关闭自动创建视图后注入自定义视图对象 |
| `USingularisInteractionSubsystem::RegisterMapping` / `UnregisterMapping` | `Subsystems/SingularisInteractionSubsystem.h` | 项目侧调用 | 运行期动态增删可命中的几何体 |
| 事件分发器（交互组件 5 个、交互者组件 2 个） | 组件头文件 | 蓝图 / C++ 订阅 | 不经过视图接口的旁路监听 |
| 资产工厂与资产行为 | `SingularisInteractionEditor/Factories/*` | 内容浏览器 | 创建策略、行为策略、查询器、交互控件的蓝图资产 |

## 七、 系列工程规范 (Series Conventions)

本节提炼自本插件的现有实现。本插件作为 Singularis 系列的参考实现，以下约定可作为新插件的一致性基线。

### 1. 三元标准模式 (Logic — Presentation — Control)

新插件按系列标准模式划分职责：逻辑端承载领域状态与权威结算，表现端只读消费状态与事件，控制端采集输入并经网络边界提交请求。依赖保持单向（控制 → 逻辑、逻辑/控制 → 表现）：表现端不写回，逻辑端不引用输入与 UI。三元的载体、契约与部署方式见 2.1。

### 2. 模块与目录结构

- 每个插件固定两个模块：运行时模块承载全部逻辑，编辑器模块承载资产工厂、资产行为与编辑器扩展；两者均 `LoadingPhase = Default`。
- 源文件按 `Public` / `Private` 镜像分目录，目录名使用领域名词复数：`Components`、`Configs`、`Factories`、`Interfaces`、`Objects`、`Subsystems`、`Types`、`Widgets`。
- 类型名与文件名严格同名（`USingularisInteractionComponent` ↔ `SingularisInteractionComponent.h/.cpp`），便于按符号定位文件。
- 模块级依赖写在 `PrivateDependencyModuleNames`（不对外暴露引擎依赖）；`PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs`。
- 头文件包含分组：引擎头用尖括号（`#include <Components/ActorComponent.h>`）靠前，插件头用引号（`#include "Components/..."`）靠后，各自内部按字母序排列。

### 3. 文件头与许可

- 每个源文件顶部为统一横幅注释块：文件名、`SPDX-License-Identifier: MIT`、`SPDX-FileCopyrightText`、版权行、创建日期与作者、MIT 许可全文。
- 插件根目录放置 `LICENSE.md` 与 `README.md`；`README.md` 当前仅含标题。

### 4. 命名规范

| 类别 | 约定 | 示例 |
| --- | --- | --- |
| 类 / 结构体 / 枚举 | 引擎前缀 + `Singularis` + 领域 + 角色 | `USingularisInteractionComponent`、`FSingularisInteractionStrategyContext`、`ESingularisInteractionMode` |
| 委托类型 | `FOn` + 语义 + `Signature`（声明）/ `Event`（成员） | `FOnInteractionSignature`、`OnInteractionEvent` |
| 模块 API 宏 | 插件名大写 + `_API` | `SINGULARISINTERACTION_API` |
| 布尔成员 | `b` 前缀 | `bIsEnabled`、`bAutoCreateView` |
| 元数据展示名 | 英文方法名 + 中文 `DisplayName` | `meta = (DisplayName = "TryInteraction")` |

### 5. 类元数据与分类

- 运行时核心类统一使用 `ClassGroup = ("Singularis")`。
- 可挂载组件使用 `meta = (BlueprintSpawnableComponent, DisplayName = "引力奇点<角色>组件")`，并在构造期显式声明 Tick 与复制策略。
- 编辑器可见属性使用中文 `Category`，并允许 `|` 子层级：`Category = "引力奇点交互组件|API"`、`Category = "引力奇点交互策略|SPI"`。

### 6. 代码组织与注释

- 头文件与实现文件使用 `#pragma region` 分区，分区顺序：`Parameter` → `Event Dispatcher` → `State` → `Constructors` → `ActorComponent Interface` → `API` → `SPI` → `RPC` → `Callback` → `Internal Function`。
- 类注释为中文段落，说明角色定位与协作对象；函数注释遵循 Doxygen 风格，含摘要与 `@param` / `@return`。
- 实现函数内部使用编号步骤注释（`// 1) ...`、`// 2) ...`）描述执行阶段，网络与安全相关注释显式标注理由。

### 7. 蓝图暴露规范

| 场景 | 标注 |
| --- | --- |
| 可被蓝图覆写的执行语义 | `BlueprintNativeEvent` + `BlueprintCallable` |
| 无副作用的查询 | `BlueprintPure` |
| 服务器限定逻辑 | `BlueprintAuthorityOnly` |
| 编辑器内联创建的对象型配置 | `UCLASS(... EditInlineNew, CollapseCategories)` + `UPROPERTY(Instanced, EditDefaultsOnly)` |
| 组件引用配置 | `FComponentReference` + `meta = (UseComponentPicker, AllowedClasses = "...")` |
| 类选择约束 | `meta = (MustImplement = "...")`、`meta = (EditCondition = "...")` |

### 8. 网络编程规范

- 需要独立 RPC 通道的组件显式 `SetIsReplicatedByDefault(true)`；纯观察者组件显式 `SetIsReplicatedByDefault(false)`。
- 子对象复制统一走 `bReplicateUsingRegisteredSubObjectList = true` + `AddReplicatedSubObject`，登记动作限定在 `HasAuthority()` 端。
- 客户端到服务器的请求使用 `Server` + `Reliable` + `WithValidation`，校验函数与实现函数分离，校验失败由引擎丢弃请求。
- 可被蓝图继承并需要 RPC 的子对象，必须覆写 `IsSupportedForNetworking`、`GetFunctionCallspace`、`CallRemoteFunction`，并沿 `GetOuter()` 链定位宿主 Actor 的网络驱动。

### 9. 防御式编程与幂等

- 所有外部输入函数以 `IsValid` 守卫开路，守卫对象包括世界、宿主 Actor、控制器、组件引用与资源指针。
- 状态写入函数先做幂等比较，再执行广播与副作用，保证重复调用不产生重复事件。
- 编辑器选择器约束（`MustImplement`、`AllowedClasses`）在运行期以 `ensureMsgf` 复核，防止蓝图与 C++ 绕过约束。
- 跨对象引用使用 `TWeakObjectPtr`（运行时缓存）或 `TObjectPtr`（`UPROPERTY` 成员），避免悬垂引用。

### 10. 内容资源规范

| 前缀 | 类别 | 示例 |
| --- | --- | --- |
| `WBP_Default_<Plugin>` | 默认控件 | `WBP_Default_SingularisInteractionWidget` |
| `IMC_Default_<Role>` | 默认输入映射上下文 | `IMC_Default_SingularisInteractor` |
| `IA_<Action>` | 输入动作 | `IA_Interaction` |
| `T_<Name>` | 贴图 | `T_Circle`、`T_Spacebar` |

内容目录按用途分组：`Inputs/`、`Inputs/Actions/`、`Textures/`、`UserInterfaces/`。

## 八、 已知约束与当前实现边界 (Known Constraints)

### 1. 未接线的声明

| 项 | 现状 |
| --- | --- |
| `USingularisInteractionStrategy::CanExecute` | 已声明并暴露给蓝图，默认返回 `true`；`TryInteraction` 直接调用 `Execute`，运行时不调用 `CanExecute` |
| `FSingularisInteractionStrategyPipeline::bSuspend` | 已声明并可在编辑器配置（默认 `true`）；当前无任何读取方 |
| `ESingularisInteractionMode` / `USingularisInteractionQueryer::SingularisInteractionMode` | 提供 `Ray` 与 `Collision` 两项；基类 `Query` 实现固定执行 `LineTraceSingleByChannel`，不读取该属性；`Collision` 模式未实现 |
| `USingularisInteractionSettings` | 已挂载 `Config = SingularisInteraction` 与项目设置分类；当前无配置字段 |

### 2. 状态与门控

- 交互组件的启用状态（`bIsEnabled`）不参与目标锁定与触发准入：`Query` 与 `TryInteraction` 均不检查该状态。其作用范围为事件广播（`OnInteractionEnableEvent` / `OnInteractionDisableEvent`）与行为策略驱动。
- 结果幂等比较只比较 `InteractionActor` 与 `InteractionComponent`，不比较 `ImpactPoint`；同一目标内的命中点变化不更新 `CurrentQueryerResult.ImpactPoint`，也不触发目标变更事件。
- 行为策略在调用 `SetEnabled` / `SetHovered` 的端就地执行，插件不复制状态、不转发副作用。

### 3. 网络行为

- `OnInteractionEvent` 在服务器（`TryInteraction`）广播；交互组件的事件分发器未标注 `Replicated`，交互控件组件在本地控制器侧订阅。因此该广播与远程客户端的订阅不在同一端，远程客户端不接收来自服务器的触发通知。
- 策略与行为策略对象经子对象复制下发，但 SPI 的执行结果不参与复制；需要跨端表现时，由策略子类自行声明 RPC（基类已提供路由支持）。

### 4. 项目级配置依赖

- 交互通道硬编码为 `ECC_GameTraceChannel1`（源码内以 `#define ECC_INTERACTION ECC_GameTraceChannel1` 在多个编译单元重复定义）。项目必须将 `GameTraceChannel1` 作为交互通道使用，并让可交互几何体对其产生 `Block` 响应；本仓库 `Config/DefaultEngine.ini` 已将其命名为 `ECC_INTERACTION`。
- 默认资产路径硬编码于构造函数：`/SingularisInteraction/Inputs/IMC_Default_SingularisInteractor`、`/SingularisInteraction/Inputs/Actions/IA_Interaction`、`/SingularisInteraction/UserInterfaces/WBP_Default_SingularisInteractionWidget`。解析失败时对应字段保持为空，不报错。

### 5. 资产与代码偏差

- `USingularisInteractorWidgetComponent` 构造函数引用 `/SingularisInteraction/UserInterfaces/WBP_Default_SingularisInteractorWidget`，该资产当前不存在于插件 `Content/UserInterfaces`（该目录仅含 `WBP_Default_SingularisInteractionWidget`）。解析失败时 `InteractorWidgetClass` 保持为空，`bAutoCreateView = true` 时不会创建视图。编辑器模块已提供 `USingularisInteractorWidget` 的资产工厂：启用自动创建视图需将同名控件蓝图放入上述路径，或在子类中指定 `InteractorWidgetClass`，或关闭自动创建并经 `SetInteractorView` 注入。
- `USingularisInteractionSubsystem` 的文件头注释块仍使用旧名 `SingularisInteractionMappingSubsystem.h`；其 `UCLASS` 上的 `BlueprintSpawnableComponent` 元数据对 `UWorldSubsystem` 无实际作用。
- `USingularisInteractionStrategy` 与 `USingularisInteractionBehaviorStrategy` 的 `GetLifetimeReplicatedProps` 仅调用 `Super`，子类自定义复制属性需自行覆写并添加 `DOREPLIFETIME`。

## 九、 关键字字典 (Keywords Glossary)

- **Logic — Presentation — Control (逻辑 — 表现 — 控制)：** Singularis 系列标准模式；逻辑端承载领域状态与权威结算，表现端（UI）只读消费状态与事件，控制端（输入）采集输入并跨网络边界提交请求，依赖单向且表现端不回写。本插件中三者分别由交互组件与策略对象、两个控件组件与视图接口、交互者组件承担（见 2.1）。
- **Interaction (交互)：** 玩家输入经由查询锁定、RPC 过桥、服务器策略执行直至表现反馈的完整通路；本插件只提供该通路的骨架。
- **Interactor (交互者)：** 玩家侧驱动组件 `USingularisInteractorComponent`，负责查询、悬浮迁移、输入绑定与请求转发；必须挂载于 `APlayerController`。
- **Queryer (查询器)：** `USingularisInteractionQueryer`，目标搜索算法单元，默认实现为单次 `LineTraceSingleByChannel`。
- **Strategy (交互策略)：** `USingularisInteractionStrategy`，触发语义的执行单元，经标签层级匹配后按数组顺序执行；业务副作用在此落地。
- **Behavior Strategy (行为策略)：** `USingularisInteractionBehaviorStrategy`，状态语义的执行单元，响应启用、禁用、悬浮、未悬浮四类边沿事件，不参与触发管线。
- **Pipeline (管线)：** `FSingularisInteractionStrategyPipeline`，一组有序策略的包装体，是映射表的值类型。
- **Tag Hierarchy Matching (标签层级匹配)：** `Tag.MatchesTag(StrategyTag)` 判定法；映射键须为触发标签的等值或后代，触发父标签会命中其子树下的全部管线。
- **Mapping Subsystem (映射子系统)：** `USingularisInteractionSubsystem`，世界级 `TMap`，把可命中的碰撞组件映射到承载交互语义的交互组件；键值均为弱引用。
- **View Interface (视图接口)：** `ISingularisInteractionViewInterface` 与 `ISingularisInteractorViewInterface`，表现层唯一依赖契约；实现者可为任意 `UObject`，不限于控件。
- **Full Pull（全量刷新）：** 绑定完成后由控件组件主动调用 `OnRefresh` 推送一次完整状态，消除绑定前错过事件导致的空白期。
- **Prompt Volume (提示范围)：** `UShapeComponent` 重叠体，仅用于进入与离开范围的反馈；其交互通道响应被强制设为 `Ignore`，不参与视线查询。
- **Server Authority (服务器权威)：** `TryInteraction` 标注 `BlueprintAuthorityOnly`，交互副作用只在服务器结算；客户端仅通过 `Server` `Reliable` RPC 发起请求。
- **Subobject Replication (子对象复制)：** 以 `bReplicateUsingRegisteredSubObjectList` + `AddReplicatedSubObject` 将 `Instanced` 策略对象纳入复制与 RPC 路由。
- **Subobject RPC Routing (子对象 RPC 路由)：** 策略基类覆写 `GetFunctionCallspace` 与 `CallRemoteFunction`，沿 Outer 链定位宿主 Actor 的网络驱动，将子对象自身作为路由对象传入，使策略子类可声明自有 RPC。
- **Instanced / EditInlineNew (内联实例化)：** 编辑器内直接创建子对象的属性声明方式，配合 `CollapseCategories` 使用，是策略与查询器的配置形态。
- **Idempotence (幂等)：** 状态写入前比较旧值、未变化即返回的约定，用于消除重复事件与重复副作用。
- **Glue (胶水层)：** 项目侧通过组件引用、策略子类与视图实现把本插件接入具体玩法的代码；插件自身不含胶水逻辑以外的业务。
