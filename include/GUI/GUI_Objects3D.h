
#ifndef SDL2_3D_ENGINE_GUI_INSPECTOR_H
#define SDL2_3D_ENGINE_GUI_INSPECTOR_H


#include "GUI.h"
#include "../../imgui/imgui.h"
#include "../Objects/Object3D.h"
#include "../Objects/Mesh3D.h"
#include "../Objects/SpriteDirectional3D.h"
#include "../Objects/Sprite3D.h"
#include "../Physics/Mesh3DBody.h"
#include "../Physics/Mesh3DGhost.h"
#include "../Render/Logging.h"
#include "../Objects/Decal.h"
#include "../Objects/Mesh3DAnimatedCollection.h"
#include "../Game/DoorGhost.h"

class GUI_Objects3D : public GUI {
public:
    bool show = true;
    int misc_flags = ImGuiColorEditFlags_NoOptions;

    virtual ~GUI_Objects3D() {}

    virtual void draw(std::vector<Object3D *> &gameObjects) {
        ImGuiWindowFlags window_flags = 0;

        if (show) {

            ImGui::SetNextWindowPos(ImVec2(2, 22), ImGuiSetCond_Once);
            ImGui::SetNextWindowSize(ImVec2(250, 825), ImGuiSetCond_Once);
            //window_flags |= ImGuiWindowFlags_NoMove;

            std::string title = "Object Inspector (" + std::to_string(gameObjects.size()) + " objects)";
            ImGui::Begin(title.c_str(), &show, window_flags);

            const float range_min = 0;
            const float range_max = 10000;
            const float range_sensibility = 0.5;

            const float range_speed_min = 0;
            const float range_speed_max = 1;
            const float range_speed_sensibility = 0.1;

            const float range_angle_min = -360;
            const float range_angle_max = 360;
            const float range_angle_sensibility = 0.1;

            const int range_framerate_min = 0;
            const int range_framerate_max = 100;

            for (int i = 0; i < gameObjects.size(); i++) {
                std::string header_text;
                Mesh3D *pMesh = dynamic_cast<Mesh3D *>(gameObjects[i]);
                if (pMesh != NULL && !gameObjects[i]->isDecal()) {
                    header_text = gameObjects[i]->getLabel() + " (numTriangles: " +
                                  std::to_string(pMesh->modelTriangles.size()) + ")" + "##" + std::to_string(i);
                } else {
                    header_text = gameObjects[i]->getLabel() + "##" + std::to_string(i);
                }

                std::string enabled_text = "Enabled##" + std::to_string(i);
                std::string position_text = "Position##" + std::to_string(i);
                std::string rotation_text = "Rotation##" + std::to_string(i);
                std::string animation_text = "Animation##" + std::to_string(i);
                std::string collection = "Collection##" + std::to_string(i);
                std::string removed_text = "Removed##" + std::to_string(i);
                std::string follow_camera_text = "FollowCamera##" + std::to_string(i);
                std::string draw_offset_text = "DrawOffset##" + std::to_string(i);

                if (ImGui::CollapsingHeader(header_text.c_str(), false)) {
                    // position
                    if (ImGui::TreeNode(position_text.c_str())) {
                        ImGui::DragScalar("X", ImGuiDataType_Float, &gameObjects[i]->getPosition().x, range_sensibility,
                                          &range_min, &range_max, "%f", 1.0f);
                        ImGui::DragScalar("Y", ImGuiDataType_Float, &gameObjects[i]->getPosition().y, range_sensibility,
                                          &range_min, &range_max, "%f", 1.0f);
                        ImGui::DragScalar("Z", ImGuiDataType_Float, &gameObjects[i]->getPosition().z, range_sensibility,
                                          &range_min, &range_max, "%f", 1.0f);
                        ImGui::TreePop();
                    }

                    // rotation
                    if (ImGui::TreeNode(rotation_text.c_str())) {

                        // position
                        bool needUpdateRotation = false;
                        ImGui::DragScalar("X", ImGuiDataType_Float, &gameObjects[i]->rotX, range_angle_sensibility, &range_angle_min,&range_angle_max, "%f", 1.0f);
                        if (ImGui::IsItemEdited())
                            needUpdateRotation = true;
                        ImGui::DragScalar("Y", ImGuiDataType_Float, &gameObjects[i]->rotY, range_angle_sensibility, &range_angle_min,&range_angle_max, "%f", 1.0f);
                        if (ImGui::IsItemEdited())
                            needUpdateRotation = true;
                        ImGui::DragScalar("Z", ImGuiDataType_Float, &gameObjects[i]->rotZ, range_angle_sensibility, &range_angle_min,&range_angle_max, "%f", 1.0f);
                        if (ImGui::IsItemEdited())
                            needUpdateRotation = true;

                        if (needUpdateRotation) {
                            gameObjects[i]->setRotation(M3::getMatrixRotationForEulerAngles(gameObjects[i]->rotX, gameObjects[i]->rotY, gameObjects[i]->rotZ));

                        }
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                                           std::to_string(gameObjects[i]->getRotation().getPitchDegree()).c_str());
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                                           std::to_string(gameObjects[i]->getRotation().getYawDegree()).c_str());
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f),
                                           std::to_string(gameObjects[i]->getRotation().getRollDegree()).c_str());
                        ImGui::TreePop();
                    }

                    // drawOffset
                    if (ImGui::TreeNode(draw_offset_text.c_str())) {
                        ImGui::DragScalar("X", ImGuiDataType_Float, &gameObjects[i]->drawOffset.x, range_sensibility,
                                          &range_min, &range_max, "%f", 1.0f);
                        ImGui::DragScalar("Y", ImGuiDataType_Float, &gameObjects[i]->drawOffset.y, range_sensibility,
                                          &range_min, &range_max, "%f", 1.0f);
                        ImGui::DragScalar("Z", ImGuiDataType_Float, &gameObjects[i]->drawOffset.z, range_sensibility,
                                          &range_min, &range_max, "%f", 1.0f);
                        ImGui::TreePop();
                    }

                    ImGui::Checkbox(removed_text.c_str(), &dynamic_cast<Object3D *>(gameObjects[i])->removed);
                    ImGui::Checkbox(follow_camera_text.c_str(),
                                    &dynamic_cast<Object3D *>(gameObjects[i])->followCamera);

                    // Only for decals
                    Decal *pDecal = dynamic_cast<Decal *>(gameObjects[i]);
                    if (pDecal != NULL) {
                        ImGui::Checkbox(std::string("Draw Decal Planes").c_str(), &pDecal->drawWireframe);
                        std::string decalNumTriangles =
                                "Decal Triangles: " + std::to_string(pDecal->modelTriangles.size());
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), decalNumTriangles.c_str());
                    }

                    // Only for SPRITES DIRECTIONALS
                    SpriteDirectional3D *pSprite3D = dynamic_cast<SpriteDirectional3D *>(gameObjects[i]);
                    if (pSprite3D != NULL) {
                        static ImGuiComboFlags flags = 0;
                        ImGui::DragScalar("Framerate", ImGuiDataType_S32,
                                          &pSprite3D->getCurrentTextureAnimationDirectional()->fps, 1.f,
                                          &range_framerate_min, &range_framerate_max, "%d fps", 1);
                        if (ImGui::IsItemDeactivatedAfterEdit()) {
                            pSprite3D->updateStep();
                        }

                        const char *items[] = {"walk", "fire", "injuried", "dead", "explosion"};
                        static const char *item_current; // Here our selection is a single pointer stored outside the object.

                        switch (pSprite3D->currentAnimation) {
                            default:
                            case 0:
                                item_current = items[0];
                                break;
                            case 1:
                                item_current = items[1];
                                break;
                            case 2:
                                item_current = items[2];
                                break;
                            case 3:
                                item_current = items[3];
                                break;
                            case 4:
                                item_current = items[4];
                                break;
                        }

                        if (ImGui::BeginCombo(animation_text.c_str(), item_current,
                                              flags)) { // The second parameter is the label previewed before opening the combo.
                            for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                                bool is_selected = (item_current == items[n]);
                                if (ImGui::Selectable(items[n], is_selected)) {
                                    item_current = items[n];
                                }

                                if (is_selected) {
                                    ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                                }

                                if (ImGui::IsItemDeactivatedAfterEdit()) {
                                    pSprite3D->currentAnimation = n;
                                    Logging::Log(("Changing animation frame to: " + std::to_string(n)), "GUI");
                                }
                            }
                            ImGui::EndCombo();
                        }
                    }

                    // Only for SPRITES
                    Sprite3D *pSprite = dynamic_cast<Sprite3D *>(gameObjects[i]);
                    if (pSprite != NULL) {
                        ImGui::DragScalar("Framerate", ImGuiDataType_S32, &pSprite->getCurrentTextureAnimation()->fps,
                                          1.f, &range_framerate_min, &range_framerate_max, "%d fps", 1);
                    }

                    // All Objects setup
                    ImGui::Checkbox(enabled_text.c_str(), &gameObjects[i]->enabled);

                    // Only for Mesh3DGhost
                    Mesh3DGhost *pMeshGhost = dynamic_cast<Mesh3DGhost *>(gameObjects[i]);
                    if (pMeshGhost != NULL) {
                        std::string counterText =
                                "currentTriggerCounter: " + std::to_string(pMeshGhost->currentTriggerCounter);
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), counterText.c_str());
                    }

                    // Only for Mesh3DBody
                    DoorGhost *pMeshBody = dynamic_cast<DoorGhost *>(gameObjects[i]);
                    if (pMeshBody != NULL) {
                        ImGui::Checkbox("Moving", &dynamic_cast<DoorGhost *>(gameObjects[i])->moving);
                        ImGui::Checkbox("Reverse moving", &dynamic_cast<DoorGhost *>(gameObjects[i])->reverseMoving);
                        ImGui::Checkbox("Waiting", &dynamic_cast<DoorGhost *>(gameObjects[i])->waiting);
                        if (pMeshBody->moving || pMeshBody->reverseMoving) {
                            std::string offsetText = "OffsetMoving :" + std::to_string(pMeshBody->offsetMoving);
                            ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), offsetText.c_str());
                        }
                    }

                    // Only for SPRITES
                    Enemy *oEnemy = dynamic_cast<Enemy *>(gameObjects[i]);
                    if (oEnemy != NULL) {
                        std::string staminaText = "Stamina :" + std::to_string(oEnemy->stamina);
                        ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), staminaText.c_str());
                    }

                    // Only for Mesh3DAnimatedCollection
                    auto *oMesh3DAnimatedCollection = dynamic_cast<Mesh3DAnimatedCollection *>(gameObjects[i]);
                    if (oMesh3DAnimatedCollection != NULL) {
                        ImGui::DragScalar("Speed", ImGuiDataType_Float,
                                          &oMesh3DAnimatedCollection->getCurrentMesh3DAnimated()->animation_speed,
                                          range_speed_sensibility, &range_speed_min, &range_speed_max, "%f", 1.0f);

                        std::string animationEndsText = "AnimationEnds: " + std::to_string(
                                oMesh3DAnimatedCollection->getCurrentMesh3DAnimated()->isAnimationEnds());
                        ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), animationEndsText.c_str());

                        static const char *item_current; // Here our selection is a single pointer stored outside the object.
                        static ImGuiComboFlags flags = 0;
                        if (oMesh3DAnimatedCollection->currentAnimation >= 0) {
                            item_current = oMesh3DAnimatedCollection->getCurrentMesh3DAnimated()->getLabel().c_str();
                        }

                        if (ImGui::BeginCombo(animation_text.c_str(), item_current,
                                              flags)) { // The second parameter is the label previewed before opening the combo.
                            for (int n = 0; n < oMesh3DAnimatedCollection->mesh3Danimated.size(); n++) {
                                bool is_selected = (item_current ==
                                                    oMesh3DAnimatedCollection->mesh3Danimated[n]->getLabel());
                                if (ImGui::Selectable(oMesh3DAnimatedCollection->mesh3Danimated[n]->getLabel().c_str(),
                                                      is_selected)) {
                                    item_current = oMesh3DAnimatedCollection->mesh3Danimated[n]->getLabel().c_str();
                                }

                                if (is_selected) {
                                    ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                                }

                                if (ImGui::IsItemDeactivatedAfterEdit()) {
                                    oMesh3DAnimatedCollection->currentAnimation = n;
                                }
                            }
                            ImGui::EndCombo();
                        }
                    }

                    const float range_potence_min = 0;
                    const float range_potence_max = 1000;
                    const float range_potence_sensibility = 0.1f;
                    // Only for meshes
                    LightPoint3D *lp = dynamic_cast<LightPoint3D *>(gameObjects[i]);
                    if (lp != NULL) {
                        bool changed_color = false;
                        bool changed_color_specular = false;
                        std::string attenuation_text = "Attenuation##" + std::to_string(i);
                        std::string colorpicker_text = "RGB##" + std::to_string(i);
                        std::string colorpicker_text_specularity = "Specularity##" + std::to_string(i);
                        std::string show_deep_map_text = "Show ZBuffer##" + std::to_string(i);
                        std::string show_frustum_map_text = "Show Frustum##" + std::to_string(i);
                        std::string specularity_RGB_text = "RGB##" + std::to_string(i);
                        if (EngineSetup::get()->CREATE_LIGHT_ZBUFFER) {
                            ImGui::Checkbox(show_deep_map_text.c_str(), &lp->showDeepMapping);
                            ImGui::Checkbox(show_frustum_map_text.c_str(), &lp->showFrustum);
                        }

                        changed_color = ImGui::ColorEdit4(colorpicker_text.c_str(), (float *) &lp->imgui_color,misc_flags);
                        if (changed_color) {
                            lp->setColor(
                                    lp->imgui_color.x * 256,
                                    lp->imgui_color.y * 256,
                                    lp->imgui_color.z * 256
                            );
                        }

                        if (ImGui::TreeNode(attenuation_text.c_str())) {
                            ImGui::DragScalar("P", ImGuiDataType_Float, &lp->p, range_potence_sensibility,
                                              &range_potence_min, &range_potence_max, "%f", 1.0f);
                            ImGui::DragScalar("Constant", ImGuiDataType_Float, &lp->kc, range_potence_sensibility,
                                              &range_min, &range_max, "%f", 1.0f);
                            ImGui::DragScalar("Linear", ImGuiDataType_Float, &lp->kl, range_potence_sensibility,
                                              &range_min, &range_max, "%f", 1.0f);
                            ImGui::DragScalar("Quadratic", ImGuiDataType_Float, &lp->kq, range_potence_sensibility,
                                              &range_min, &range_max, "%f", 1.0f);
                            ImGui::TreePop();
                        }

                        if (ImGui::TreeNode(colorpicker_text_specularity.c_str())) {
                            ImGui::DragScalar("m", ImGuiDataType_Float, &lp->specularComponent, range_potence_sensibility,
                                              &range_potence_min, &range_potence_max, "%f", 1.0f);
                            changed_color_specular = ImGui::ColorEdit4(specularity_RGB_text.c_str(), (float *) &lp->imgui_color_specularity,misc_flags);
                            if (changed_color_specular) {
                                lp->setColorSpecularity(
                                        lp->imgui_color_specularity.x * 256,
                                        lp->imgui_color_specularity.y * 256,
                                        lp->imgui_color_specularity.z * 256
                                );
                            }
                            ImGui::TreePop();
                        }
                    }
                }
            }

            ImGui::End();
        }
    }
};


#endif //SDL2_3D_ENGINE_GUI_INSPECTOR_H
