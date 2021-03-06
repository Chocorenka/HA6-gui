#include "main_pane.h"
#include <imgui.h>	

/* static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
 */

MainPane::MainPane(Render* render_):
frameData(nullptr),
currentPattern(-1),
currFrame(0),
decoratedNames(nullptr),
render(render_)
{
	
}

void MainPane::RegenerateNames()
{
	delete[] decoratedNames;
	
	if(frameData)
	{
		decoratedNames = new std::string[frameData->get_sequence_count()];
		int count = frameData->get_sequence_count();

		for(int i = 0; i < count; i++)
		{
			decoratedNames[i] = frameData->GetDecoratedName(i);
		}
	}
	else
		decoratedNames = nullptr;
}

void MainPane::SetFrameData(FrameData *frameData_)
{
	if(frameData_)
		currentPattern = 0;
	else
		currentPattern = -1;

	frameData = frameData_;

	RegenerateNames();
}


void MainPane::Draw()
{	
	ImGui::Begin("Left Pane",0 , ImGuiWindowFlags_NoMove );
	if(frameData->m_loaded)
	{
		if (ImGui::BeginCombo("Pattern", decoratedNames[currentPattern].c_str(), ImGuiComboFlags_HeightLargest))
		{
			auto count = frameData->get_sequence_count();
			for (int n = 0; n < count; n++)
			{
				//std::string &&name = std::to_string(n) + " " + frameData->GetName(n); 
				const bool is_selected = (currentPattern == n);
				if (ImGui::Selectable(decoratedNames[n].c_str(), is_selected))
				{
					currentPattern = n;
					currFrame = 0;
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		auto seq = frameData->get_sequence(currentPattern);
		if(seq)
		{
			if (ImGui::TreeNode("Pattern data"))
			{
				constexpr float spacing = 80;
				float width = 100;
				float height = ImGui::GetTextLineHeightWithSpacing()*5.5;

				ImGui::BeginChild("child", ImVec2(width, height), true);
				ImGui::Text("Hitboxes:"); ImGui::SameLine(spacing);
				ImGui::Text("%llu", seq->hitboxes.size());
				
				ImGui::Text("nAT:"); ImGui::SameLine(spacing);
				ImGui::Text("%llu", seq->AT.size());

				ImGui::Text("nAS:"); ImGui::SameLine(spacing);
				ImGui::Text("%llu", seq->AS.size());

				ImGui::Text("nEF:"); ImGui::SameLine(spacing);
				ImGui::Text("%llu", seq->EF.size());

				ImGui::Text("nIF:"); ImGui::SameLine(spacing);
				ImGui::Text("%llu", seq->IF.size());
				ImGui::EndChild();

				constexpr float ptcnSpacing=22.f;
				ImGui::SameLine();
				ImGui::BeginChild("child2", ImVec2(ImGui::GetWindowWidth()-width-60,height), true, ImGuiWindowFlags_HorizontalScrollbar);
				ImGui::SetNextItemWidth(ptcnSpacing);
				ImGui::InputScalarN("##PTCN1", ImGuiDataType_S8, &seq->ptcn[0], 1, NULL, NULL, "%d");
				ImGui::SetNextItemWidth(ptcnSpacing); ImGui::SameLine();
				ImGui::InputScalarN("##PTCN2", ImGuiDataType_S8, &seq->ptcn[1], 1, NULL, NULL, "%d");
				ImGui::SetNextItemWidth(ptcnSpacing); ImGui::SameLine();
				ImGui::InputScalarN("##PTCN3", ImGuiDataType_S8, &seq->ptcn[2], 1, NULL, NULL, "%d");
				ImGui::SetNextItemWidth(ptcnSpacing); ImGui::SameLine();
				ImGui::InputScalarN("##PTCN4", ImGuiDataType_S8, &seq->ptcn[3], 1, NULL, NULL, "%d");
				ImGui::SetNextItemWidth(ptcnSpacing); ImGui::SameLine();
				ImGui::InputScalarN("PTCN", ImGuiDataType_S8, &seq->ptcn[4], 1, NULL, NULL, "%d");
				
				ImGui::SetNextItemWidth(40.f);
				ImGui::InputInt("PSTS", &seq->psts, 0, 0);

				ImGui::SetNextItemWidth(40.f);
				ImGui::InputInt("Level", &seq->level, 0, 0);

				ImGui::SetNextItemWidth(80.f);
				ImGui::InputInt("Flag (PFLG)", &seq->flag, 0, 0);

				ImGui::EndChild();

				ImGui::TreePop();
				ImGui::Separator();
			}

			int nframes = seq->frames.size() - 1;
			if(nframes >= 0)
			{			
				float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 160.f);
				ImGui::SliderInt("##frameSlider", &currFrame, 0, nframes);
				ImGui::SameLine();
				ImGui::PushButtonRepeat(true);
				if (ImGui::ArrowButton("##left", ImGuiDir_Left) && currFrame > 0) {currFrame--;}
				ImGui::SameLine(0.0f, spacing);
				if (ImGui::ArrowButton("##right", ImGuiDir_Right) && currFrame < nframes) {currFrame++;}
				ImGui::PopButtonRepeat();
				ImGui::SameLine();
				ImGui::Text("%d/%d", currFrame+1, nframes+1);

				Frame &frame = seq->frames[currFrame];
				if(frame.AF.active)
				{
					spriteId = frame.AF.frame;
					render->offsetX = (frame.AF.offset_x-128)*2;
					render->offsetY = (frame.AF.offset_y-224)*2;
				}
				else
				{
					spriteId = -1;	
				}

				render->GenerateHitboxVertices(frame.hitboxes, frame.nHitbox);
			}
			else
			{
				spriteId = -1;
				ImGui::Text("This pattern has no frames.");
				render->DontDraw();
			}


		}
		else
		{
			spriteId = -1;
			ImGui::Text("This pattern is empty.");
			render->DontDraw();
		}
	}
	else
		render->DontDraw();




	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();

	render->SwitchImage(spriteId);
	
}

void MainPane::ForceUpdate()
{
	Sequence *seq;
	Frame *frame;
	if(	frameData->m_loaded &&
		(seq = frameData->get_sequence(currentPattern)) &&
		seq->frames.size() > 0)
	{
		auto &frame =  seq->frames[currFrame];
		spriteId = frame.AF.frame;
		render->GenerateHitboxVertices(frame.hitboxes, frame.nHitbox);
		render->offsetX = (frame.AF.offset_x-128)*2;
		render->offsetY = (frame.AF.offset_y-224)*2;
	}
	else
	{
		spriteId = -1;
		
		render->DontDraw();
	}
	render->SwitchImage(spriteId);
}

void MainPane::AdvancePattern(int dir)
{
	currentPattern+= dir;
	if(currentPattern < 0)
		currentPattern = 0;
	else if(currentPattern >= frameData->get_sequence_count())
		currentPattern = frameData->get_sequence_count()-1;
	currFrame = 0;

	ForceUpdate();
}