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
#include "Core/Transform.h"
#include "Core/SceneManager.h"
#include "Core/PersistentManager.h"
#include "Core/MemoryManager.h"
#include "Core/FileSystem.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"

//-----------------Physics-----------------//
#include "Physics/Rigidbody.h"
#include "Physics/SphereCollider.h"
#include "Physics/BoxCollider.h"
#include "Physics/CapsuleCollider.h"
#include "Physics/CylinderCollider.h"
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
#include "ECS/Components/DirectPathfinding.h"
#include "ECS/Components/Light.h"
#include "ECS/Components/MeshRenderer.h"
#include "ECS/Components/Particle.h"
#include "ECS/Components/ScriptComponent.h"
#include "ECS/Components/Slideshow.h"
#include "ECS/Components/Sprite3DComponent.h"
#include "ECS/Components/TextComponent.h"
#include "ECS/Components/UIComponent.h"

//Components that i havent move: (i will move)
// Prefab
// Parenting
// Properties
// Transform
// Camera
// Rigidbody
// Boxcollider
// capsulecollider
// cylindercollier