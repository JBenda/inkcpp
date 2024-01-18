#pragma once

#include "Logging/LogMacros.h"

/**
 * @defgroup unreal Unreal Blueprints
 * Blueprint Classes usable in Unreal. An example can be found
 * [here](unreal/inkCPP_DEMO.zip), do not forgett to install the plugin via the marketplace(soonTM) or 
 * unzipping the `unreal.zip` from the [release page]() to `/UNREAL_ENGINE/Engine/Plugins`. <br/>
 * And eitherway activating the plugin.
 *
 * The C++ API will be available soon(TM).
 *
 * @section ue_example The Example project
 *
 * [Download](unreal/inkCPP_DEMO.zip)
 *
 * The example contains two maps:
 * + @ref ue_example_demo "`Demo`": An extensive example demonstrating many but not all features provided by inkCPP like:
 *   + snapshots: for creating save games
 *   + observers: to easily refelct a variable of the ink story in the game.
 *   + external function + yield: to stop playing the story while the game plays a transition
 *   + a second runner: for a inventory menu
 *   + interopariblity between UE Enums and Lists in Ink
 *   + Tag attributes: use tags to modify showed text
 * + @ref ue_example_minimal "`Minimal`": An example for a minimal, still sensible usage example of InkCPP in UE5.
 *
 * @subsection ue_exapmle_demo  Demo
 *
 * @subsection ue_example_minimal Minimal
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
