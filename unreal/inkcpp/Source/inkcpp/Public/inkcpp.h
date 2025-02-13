/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once
#include "Logging/LogMacros.h"

/**
 * @defgroup unreal Unreal Blueprints
 * Blueprint Classes usable in Unreal. An example can be found
 * [here](../unreal/InkCPP_DEMO.zip), do not forget to install the plugin via the
 * [marketplace](https://www.unrealengine.com/marketplace/product/inkcpp) or unzipping the
 * `unreal_X_x.zip` from the [release page](https://github.com/JBenda/inkcpp/releases/latest) to
 * `/AN_TEMP_DIRECTORY/` and build it with:
 * ```sh
 * /UNREAL_ENGINE/Build/BatchFiles/RunUAT.bat BuildPlugin
 * -plugin=/AN_TEMP_DIRECTORY/inkcpp/inkcpp.uplugin -package=/YOUR_UNREAL_PROJECT/Plugins/inkcpp
 * -TargetPlatforms=Win64`
 * ``` <br/> And either way activating the plugin.
 *
 * The C++ API will be available soon([Issue](https://github.com/JBenda/inkcpp/issues/60)).
 *
 * + @ref ue_setup "General setup"
 * + @ref ue_components "UE5 Blueprints"
 * + @ref ue_example "Demo project & setup"
 *
 * @section ue_setup Setup
 *
 * After installing the plugin (see above) you need to activate it via `Plugins->...`.
 * Then to run your Ink script you need a instance of @ref AInkRuntime. The story will
 * only proceed if this actor is active.
 *
 * On this instance set the `Ink|InkAsset` property to the story that you will run.
 * To create this InkAsset you need to import a `.ink` file or `.ink.json` file.
 *
 * With the @ref AInkRuntime you can then create a @ref UInkThread with @ref AInkRuntime::Start().
 * In addition does the runtime allows you access to the global variables via @ref
 * AInkRuntime::ObserverVariableChange() "observer" or directly @ref
 * AInkRuntime::SetGlobalVariable() "setter" und @ref AInkRuntime::GetGlobalVariable() "getter".
 *
 * Notice that all threads spawned in the
 * same runtime will share a global state. So if you want to play the same story with different
 * states, you need multiple runtimes.
 *
 * The @ref UInkThread class provides will fire events when new context is available. The easiest
 * way to implement then is to create a custom Blueprint based on @ref UInkThread. For a overview
 * see @ref ue_thread.
 *
 * Below you can find the blueprints of a  @ref ue_example_minimal "minimal example". Which is
 * included for further inspection inside the @ref ue_example "Example project" (in the map
 * `Minimal`).
 *
 * @section ue_components Components
 *
 * @subsection ue_runtime Runtime
 * The @ref AInkRuntime handles the runtime as actor.
 * At every Tick all @ref UInkThread of the runtime will be further executed if appropriate.
 *
 * The asset containing the story to run can be set via the Ink|InkAsset attribute.
 *
 * The runtime is the position to set observer (e.g. @ref AInkRuntime::ObserverVariableChange() )
 * and create new threads  (@ref AInkRuntime::Start() & @ref AInkRuntime::StartExisting() ).
 *
 * It is advised to create you own Blueprint which inherits @ref AInkRuntime to overwrite the
 * events as necessary.
 *
 * @subsection ue_thread Thread
 * A @ref UInkThread is like a pointer inside the story. It contains information can advance and
 * will thereby output the context it encounters.
 *
 * All threads inside the same runtime will share the same variables but can be at different points
 * in the story.
 *
 * The most important events/functions are:
 * + @ref UInkThread::OnLineWritten() which is called by each new line of output
 * + @ref UInkThread::OnChoice() which is called if a choice is encountered and must be handled
 * + @ref UInkThread::PickChoice()  to pick a choice and continue the thread.
 *
 * @subsection ue_choice Choice
 * A @ref UInkChoice contains all data relevant for a branch at a choice point.
 * + @ref UInkChoice::GetText() the text of the choice
 * + @ref UInkChoice::GetIndex() the index used in @ref UInkThread::PickChoice()
 * + @ref UInkChoice::GetTags() tags associated with this branch/choice
 *
 * @subsection ue_taglist TagList
 * A @ref UTagList is a wrapper for the array of tags each line of context and each choice can have.
 *
 * @subsection ue_ivar InkVar
 * A wrapper for variables of the ink runtime.
 * To get/set variables you need access to the runtime:
 * @ref AInkRuntime::GetGlobalVariable(), @ref AInkRuntime::SetGlobalVariable()
 *
 * please note that get returns a copy of the variables value and to change it you need to call set.
 *
 * @subsection ue_list InkList
 * @ref UInkList is a wrapper for the list type inside ink.
 * A ink list is like a set for enum values. For a in depth explanation please refer to the [official
 * guide](https://blueprintue.com/blueprint/hdybtdjp/)
 *
 * If you define Enums similar to the Lists in the ink script you can use them for an easier access.
 *
 * @section ue_example The Example project
 *
 * [Download](../unreal/InkCPP_DEMO.zip)
 *
 * @subsection ue_example_setup Setup
 *
 * To setup the [example project](../unreal/InkCPP_DEMO.zip) install the Plugin via the [UE
 * marketplace](https://www.unrealengine.com/product/inkcpp) place unpack
 * the `unreal_X_x.zip` from the [release page](https://github.com/JBenda/inkcpp/releases/latest)
 * inside
 * `/PATH/InkCPP_DEMO/Plugins/`.
 *
 * Next open the project via the `InkCPP_DEMO/InkCPP_DEMO.uproject` file.
 *
 * Than you can hit play to run the demo.
 *
 * The example contains two maps:
 * + @ref ue_example_demo "`Demo`": An extensive example demonstrating many but not all features
 * provided by inkCPP like:
 *   + snapshots: for creating save games
 *   + observers: to easily reflect a variable of the ink story in the game.
 *   + external function + yield: to stop playing the story while the game plays a transition
 *   + a second runner: for a inventory menu
 *   + interoperability between UE Enums and Lists in Ink
 *   + Tag attributes: use tags to modify showed text
 *   + Tag attributes as enums: use tags to modify choices
 * + @ref ue_example_minimal "`Minimal`": An example for a minimal, still sensible usage example of
 * InkCPP in UE5.
 *
 * @subsection ue_example_demo  Demo
 *
 * The Demo map contains the Actor `DemoThread` which is a child of `InkRuntime`.
 * All UI elements and other used components are created on the `BeginPlay` event in the following
 * order.
 * 1. The UI components are created and configured
 * 2. Load an existing save game if its exists (the Save game is stored at
 * `InkCPP_DEMO/Saved/SaveGames`).
 * 3. Create the main thread of class `DemoThread` and register the external function.
 * 4. Create menu thread(`InfoThread`), set path to `Wait` to avoid any output in the beginning.
 * 5. Set observer for the variable `Heath` to update the healthbar.
 * 6. Set observer for the variable `Inventory` to update the inventory columns.
 *
 * @subsubsection ue_example_ui UI
 *
 * + `DialogHUD` contains all static UI elements.
 *   + `Context`: text box containing the lines of the ink story.
 *   + `Choices`: A container which should be populated with the choice buttons
 *   + `Clues` & `Potions`: container which should be populated with inventory buttons
 *   + `SC_Button`: Button to trigger save and close action
 *   + `Health`: health bar showing current health
 *   + `DMG_Numbers`: container which should be populated with damage numbers
 *   + `Popup`/`PopupContext`/`PopopChoices`: elements needed for the Info/"Item interaction thread"
 *   + `TransitionBlob`: A animated entity used to simulate a transition.
 * + `DMG_Number` animated text block used to display damage numbers
 * + `InventoryButton`/`ChoiceButton`: Wrapper for buttons, primarily for attaching data
 *    to a button for a parameterized clicked event.
 *
 * @subsubsection ue_example_demo_DemoRunner DemoRunner
 *
 * @htmlonly
 * <a href="https://blueprintue.com/blueprint/owj83khu/">
 * <img
 * alt="Thread Creation Blueprint"
 * src="../unreal/imgs/CreateThread.png"
 * width="80%"/></a>
 * @endhtmlonly
 *
 * @htmlonly
 * <a href="https://blueprintue.com/blueprint/7bjjjb6u/">
 * <img
 * alt="Observe Change of variable Blueprint"
 * src="../unreal/imgs/ObseverChange.png"
 * width="80%"/></a>
 * @endhtmlonly
 *
 * @htmlonly
 * <a href="https://blueprintue.com/blueprint/hdybtdjp/">
 * <img
 * alt="Usage example for InkList::ElementOf in a Blueprint"
 * src="../unreal/imgs/ListElementOf.png"
 * width="80%"/></a>
 * @endhtmlonly
 *
 * <a href="https://blueprintue.com/blueprint/mf-hwyg5/">
 * <img
 * alt="A InkThread Yield Resume example Blueprint"
 * src="../unreal/imgs/YieldResume.png"
 * width="80%"/></a>
 *
 * @subsubsection ue_example_demo_DemoThread DemoThread
 *
 * <a href="https://blueprintue.com/blueprint/q8wep7r6/">
 * <img
 * alt="Example of the ussage of TagList::GetValue inside processing a new context line."
 * src="../unreal/imgs/TagListGetValue.png"
 * width="80%"/></a>
 *
 * <a href="https://blueprintue.com/blueprint/r5jbthpn/">
 * <img
 * alt="Example for choice handling."
 * src="../unreal/imgs/HandleChoice.png"
 * width="80%"/></a>
 *
 * @subsection ue_example_minimal Minimal
 *
 * <a href="https://blueprintue.com/blueprint/712hsqyl/">
 * <img
 * alt="Minmal InkRuntime Blueprint"
 * src="../unreal/imgs/MinimalRuntime.png"
 * width="80%"/></a>
 *
 * <a href="https://blueprintue.com/blueprint/-da0bqy5/">
 * <img
 * alt="Minimal InkThread Blueprint"
 * src="../unreal/imgs/MinimalThread.png"
 * width="80%"/></a>
 *
 */

DECLARE_LOG_CATEGORY_EXTERN(InkCpp, Log, All);
