#pragma once

#include "Logging/LogMacros.h"

/**
 * @defgroup unreal Unreal Blueprints
 * Blueprint Classes usable in Unreal. An example can be found
 * [here](unreal/inkCPP_DEMO.zip), do not forgett to install the plugin via the marketplace(soonTM)
 * or unzipping the `unreal.zip` from the [release page]() to `/UNREAL_ENGINE/Engine/Plugins`. <br/>
 * And eitherway activating the plugin.
 *
 * The C++ API will be available soon(TM).
 *
 * In the following the [Demo project](../unreal/inkCPP_DEMO.zip) will be @ref ue_example
 * "shown".<br/>
 * But firsta more general explenation of the @ref ue_components "UE5 Blueprits"
 *
 * @section ue_components Components
 *
 * @subsection ue_runtime Runtime
 * The @ref AInkRuntime handles the runtime as actor.
 * At every Tick all @ref UInkThread of the runtime will be further executed if appropiate.
 *
 * The asset containing the story to run can be set via the Ink|InkAsset attribute.
 *
 * The runtime is the position to set observer (e.g. @ref AInkRuntime::ObserverVariableChange() )
 * and create new threads  (@ref AInkRuntime::Start() & @ref AInkRuntime::StartExisting() ).
 *
 * It is adviced to create you own Blueprint which inherites @ref AInkRuntime to overwrite the
 * events as necessary.
 *
 * @subsection ue_thread Thread
 * A @ref UInkThread is like a pointer inside the story. It contains informations can advance and
 * will therby output the context it encounters.
 *
 * All threads inside the same runtime will share the same variables but can be at different points
 * in the story.
 *
 * The most importent events/functions are:
 * + @ref UInkThread::OnLineWritten() which is called by each new line of output
 * + @ref UInkThread::OnLineWritten() which is called if a choice is encounterd and must be handled
 * + @ref UInkThread::PickChoice()  to pick a choice and continue the thread.
 *
 * @subsection ue_choice Choice
 * A @ref UInkChoice contains all data relevant for a branch at a choice point.
 * + @ref UInkChoice::GetText() the text of the choice
 * + @ref UInkChoice::GetIndex() the index used in @ref UInkThread::PickChoice()
 * + @ref UInkChoice::GetTags() tags assoziated with this branch/choice
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
 * A ink list is like a set for enum values. For a in depth explenation please refer to the [offical
 * guide](https://blueprintue.com/blueprint/hdybtdjp/)
 *
 * If you define Enums simular to the Lists in the ink script you can use them for an easier access.
 *
 * @section ue_example The Example project
 *
 * [Download](../unreal/inkCPP_DEMO.zip)
 *
 * The example contains two maps:
 * + @ref ue_example_demo "`Demo`": An extensive example demonstrating many but not all features
 * provided by inkCPP like:
 *   + snapshots: for creating save games
 *   + observers: to easily refelct a variable of the ink story in the game.
 *   + external function + yield: to stop playing the story while the game plays a transition
 *   + a second runner: for a inventory menu
 *   + interopariblity between UE Enums and Lists in Ink
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
 * 2. Load an existing save game if its exists.
 * 3. Create the main thread of class `DemoThread` and register the external function.
 * 4. Create menu thread(`InfoThread`), set path to `Wait` to avoid any output in the beginging.
 * 5. Set observer for the variable `Heath` to update the healthbar.
 * 6. Set observer for the variable `Inventory` to update the inventory columns.
 *
 * @subsubsection ue_example_ui UI
 *
 * + `DialogHUD` contains all static UI elements.
 *   + `Context`: text box containing thelines of the ink story.
 *   + `Choices`: A container which should be populated with the choice buttons
 *   + `Clues` & `Potions`: container which should be populated with inventory buttons
 *   + `SC_Button`: Button to triggern save and close action
 *   + `Health`: health bar showing current health
 *   + `DMG_Numbers`: container which should be populated with damage numbers
 *   + `Popup`/`PopupContext`/`PopopChoices`: elements needed for the Info/"Item interaction thread"
 *   + `TransitionBlob`: A animated entity used to simulate a transition.
 * + `DMG_Number` animated text block used to display damage numbers
 * + `InventoryButton`/`ChoiceButton`: Wrapper for buttons, primarly for attaching data
 *    to a button for a parametrized clicked event.
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
