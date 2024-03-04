#pragma once

//-----------------MATH-----------------//
#include "Mathf.h"
#include "Easing.h"
#include "Random.h"
#include "Vector2.h"
#include "Vector2Int.h"
#include "Vector3.h"
#include "Vector3Int.h"

//-----------------CORE-----------------//
#include "Core/Engine.h"
#include "Core/System.h"
#include "Core/ECS.h"
#include "Core/EntityCopy.h"
#include "Core/Prefab.h"
#include "Core/Parent.h"
#include "Core/SceneManager.h"
#include "Core/PersistentManager.h"
#include "Core/MemoryManager.h"
#include "Core/FileSystem.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"

//-----------------Physics-----------------//
#include "Physics/PhysicsSystem.h"

//-----------------Graphics-----------------//
#include "Graphics/RendererContext.h"
#include "Graphics/SwapChain.h"
#include "Graphics/Camera.h"

//-------------------Audio-------------------//
#include "Audio/AudioSystem.h"

//-------------------Components-------------------//
#include "ECS/Components/AnimationComponent.h"
#include "ECS/Components/AudioComponent.h"
#include "ECS/Components/AudioListenerComponent.h"
#include "ECS/Components/BoxCollider.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/CapsuleCollider.h"
#include "ECS/Components/CylinderCollider.h"
#include "ECS/Components/DirectPathfinding.h"
#include "ECS/Components/Light.h"
#include "ECS/Components/MeshRenderer.h"
#include "ECS/Components/ParentingComponent.h"
#include "ECS/Components/Particle2DComponent.h"
#include "ECS/Components/Particle3DComponent.h"
#include "ECS/Components/Rigidbody.h"
#include "ECS/Components/ScriptComponent.h"
#include "ECS/Components/Slideshow.h"
#include "ECS/Components/SphereCollider.h"
#include "ECS/Components/Sprite3DComponent.h"
#include "ECS/Components/TextComponent.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/UIComponent.h"