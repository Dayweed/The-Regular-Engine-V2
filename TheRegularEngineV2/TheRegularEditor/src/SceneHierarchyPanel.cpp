#include "SceneHierarchyPanel.h"
#include "Imgui/imgui.h"

namespace TRE
{
	SceneHierarchyPanel::SceneHierarchyPanel()
	{

	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{

	}

	void SceneHierarchyPanel::Init()
	{

	}

	void SceneHierarchyPanel::Update()
	{
        ImGui::Begin("Hierarchy");

		//std::cout << "size of vector: " << ECSManager::Instance().GetEntities<Properties>().size() << "\n";
        for (size_t i{}; i < ECSManager::Instance().GetEntities<Properties>().size(); ++i)
        {
            //std::cout << i /*<< "\twhats the name: " << ECSManager::Instance().GetEntities<Properties>()[i]->GetComponent<Properties>().m_Name */<< "\n";
            
            //std::cout << "do i have children?\t" << ECSManager::Instance().GetEntities<Properties>()[i]->GetChildren().size() << "\n";
            if (i == 8 && ECSManager::Instance().GetEntities<Properties>()[i]->GetChildren().size() == 0)
            {
                Entity CHILD1 = ECSManager::Instance().CreateEntity("CHILD1");
                Entity CHILD2 = ECSManager::Instance().CreateEntity("CHILD2");
                ECSManager::Instance().GetEntities<Properties>()[i]->AddChild(CHILD1);
                ECSManager::Instance().GetEntities<Properties>()[i]->AddChild(CHILD2);
            }
        }

        if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_OpenOnArrow))
        {
            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow; //1000 0000 in binary
            ////static bool test_drag_and_drop = true;

            // 'selection_mask' is dumb representation of what may be user-side selection state.
            //  You may retain selection state inside or outside your objects in whatever format you see fit.
            // 'object_clicked' is temporary storage of what node we have clicked to process selection at the end
            /// of the loop. May be a pointer to your own node type, etc.
            static int selection_mask = 0; //nothing is selected in the beginning
            int object_clicked = -1; //none of the objects are selected
            //for (int i = 0; i < 6; ++i)
            for (size_t i{}; i < ECSManager::Instance().GetEntities<Properties>().size(); ++i)
            {
                //std::cout << i << "\t child:" << ECSManager::Instance().GetEntities<Properties>()[i]->GetChildren().size() << "\n";

                // Disable the default "open on single-click behavior" + set Selected flag according to our selection.
                // To alter selection we use IsItemClicked() && !IsItemToggledOpen(), so clicking on an arrow doesn't alter selection.
                ImGuiTreeNodeFlags node_flags = base_flags;
                const bool is_selected = (selection_mask & (1 << i)) != 0; //when any entity is selected, is_selected will be true
                if (is_selected) node_flags |= ImGuiTreeNodeFlags_Selected;

                // Items 3..5 are Tree Leaves
                // The only reason we use TreeNode at all is to allow selection of the leaf. Otherwise we can
                // use BulletText() or advance the cursor by GetTreeNodeToLabelSpacing() and call Text().

                const char* object_name = ECSManager::Instance().GetEntities<Properties>()[i]->GetComponent<Properties>().m_Name.c_str();

                bool node_open = false;
                if (ECSManager::Instance().GetEntities<Properties>()[i]->GetParent() == nullptr) //show parents/solo entities only
                {
                    if (!ECSManager::Instance().GetEntities<Properties>()[i]->GetChildren().size())
                        node_flags |= ImGuiTreeNodeFlags_Leaf;
                    node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, object_name, i);
                }

                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                    object_clicked = (int)i;

                if (/*test_drag_and_drop && */ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
                    ImGui::Text("This is a drag and drop source");
                    ImGui::EndDragDropSource();
                }

                if (node_open)
                {
                    if (ECSManager::Instance().GetEntities<Properties>()[i]->GetChildren().size() != 0) //if children exist SHOW THEM
                    {
                        ImGui::Text("how to do recurssion for child's child's child's.......");
                    }

                    ImGui::TreePop();
                }
                //if (ImGui::BeginDragDropTarget())
                //{
                //    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE"))
                //    {
                //        IM_ASSERT(payload->DataSize == sizeof(int));
                //        int payload_n = *(const int*)payload->Data;
                //        
                //        names[n] = names[payload_n];
                //        names[payload_n] = "";
                //    }
                //    ImGui::EndDragDropTarget();
                //}
            }

            if (object_clicked != -1)
            {
                // Update selection state
                // (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
                if (ImGui::GetIO().KeyCtrl)
                {
                    //when user presses on cltr and chooses more than 1 entity
                    selection_mask ^= (1 << object_clicked);          // CTRL+click to toggle
                    std::cout << "selection_mask: " << selection_mask  << "\n";
                }
                else //if (!(selection_mask & (1 << object_clicked))) // Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
                {
                    //when user clicked on an entity
                    selection_mask = (1 << object_clicked);           // Click to single-select
                }
            }
            ImGui::TreePop();
        }

		ImGui::End();
	}

	void SceneHierarchyPanel::Shutdown()
	{

	}
}