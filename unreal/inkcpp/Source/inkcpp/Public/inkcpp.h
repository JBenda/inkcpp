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
 * "explained".<br/> Followed by a more general explenation of the @ref ue_components "UE5
 * Blueprits"
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
 * [![Thread Creation
 * Blueprint](../unreal/imgs/CreateThread.png)](https://blueprintue.com/blueprint/owj83khu/
 *
 * [![Observe Change of variable
 * Blueprint](../unreal/imgs/ObseverChange.png)](https://blueprintue.com/blueprint/7bjjjb6u/)
 *
 * [![Usage example for InkList::ElementOf in a Blueprint](../unreal/imgs/ListElementOf.png)]
 *
 * [![A InkThread Yield Resume example
 * Blueprint](../unreal/imgs/YieldResume.png)](https://blueprintue.com/blueprint/mf-hwyg5/)
 *
 * @subsubsection ue_example_demo_DemoThread DemoThread
 *
 * [![Example of the ussage of TagList::GetValue inside processing a new context
 * line.](../unreal/imgs/TagListGetValue.png)](https://blueprintue.com/blueprint/q8wep7r6/)
 *
 * [![Example for choice
 * handling.](../unreal/imgs/HandleChoice.png)](https://blueprintue.com/blueprint/r5jbthpn/)
 *
 * @subsection ue_example_minimal Minimal
 *
 * [![Minmal InkRuntime
 * Blueprint](../unreal/imgs/MinimalRuntime.png)](https://blueprintue.com/blueprint/712hsqyl/)
 *
 * [![Minimal InkThread
 * Blueprint](../unreal/imgs/MinimalThread.png)](https://blueprintue.com/blueprint/-da0bqy5/)
 *
 * @section ue_components Components
 * @subsection ue_runtime
 * @subsection ue_thread
 * @subsection ue_choice
 * @subsection ue_ivar
 * @subsection ue_list
 * @subsection ue_taglist
 */

DECLARE_LOG_CATEGORY_EXTERN(InkCpp, Log, All);
