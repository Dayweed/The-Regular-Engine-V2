#pragma once
#include "Pipeline.h"
#include "RenderPass.h"
#include "AnimationStructure.h"
#include "Material.h"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace TRE
{	
    struct AnimationUBO;
    struct anim_player
    {
        //-----------------------------------------------------------------------------------------------------------

        void PlayAnim(int i) noexcept
        {
            assert(i < m_AnimPackage.m_Animations.size());
            m_iCurAnim = i;
            m_Time = 0;
        }

        void Update(float DT) noexcept
        {
            auto& Anim = m_AnimPackage.m_Animations[m_iCurAnim];

            // advance time
            m_Time += DT;
            while (m_Time >= Anim.m_TimeLength) m_Time -= Anim.m_TimeLength;
        }

        //-----------------------------------------------------------------------------------------------------------

        static float Dot(glm::quat Start, glm::quat Q)
        {
            return  Start.x * Q.x +
                    Start.y * Q.y +
                    Start.z * Q.z +
                    Start.w * Q.w;
        }

        template< typename T_PARENT, typename T_BASIC_TYPE >
        struct type
        {
            using basic_t = T_BASIC_TYPE;
            using self_t = type;
            T_BASIC_TYPE m_Value;

            constexpr                               type(void)                        noexcept = default;
            constexpr explicit                      type(basic_t Value)               noexcept : m_Value{ Value } {}
            constexpr               bool            operator == (const T_PARENT X)    const   noexcept { return m_Value == X.m_Value; }
            constexpr               bool            operator != (const T_PARENT X)    const   noexcept { return m_Value != X.m_Value; }
            constexpr               bool            operator >= (const T_PARENT X)    const   noexcept { return m_Value >= X.m_Value; }
            constexpr               bool            operator <= (const T_PARENT X)    const   noexcept { return m_Value <= X.m_Value; }
            constexpr               bool            operator >  (const T_PARENT X)    const   noexcept { return m_Value > X.m_Value; }
            constexpr               bool            operator <  (const T_PARENT X)    const   noexcept { return m_Value < X.m_Value; }
            auto            operator ++ (int)                         noexcept { T_PARENT temp{ *this }; m_Value += 1; return temp; }
            auto& operator ++ (void)                        noexcept { m_Value += 1; return *static_cast<T_PARENT*>(this); }
            auto            operator -- (int)                         noexcept { T_PARENT temp{ *this }; m_Value -= 1; return temp; }
            auto& operator -- (void)                        noexcept { m_Value -= 1; return *this; }
            auto& operator += (const T_PARENT X)            noexcept { m_Value += X.m_Value; return *static_cast<T_PARENT*>(this); }
            auto& operator -= (const T_PARENT X)            noexcept { m_Value -= X.m_Value; return *static_cast<T_PARENT*>(this); }
            auto& operator *= (const T_PARENT X)            noexcept { m_Value *= X.m_Value; return *static_cast<T_PARENT*>(this); }
            auto& operator /= (const T_PARENT X)            noexcept { m_Value /= X.m_Value; return *static_cast<T_PARENT*>(this); }
            template< typename = typename std::enable_if_t< std::is_signed<basic_t>::value, type > >
            constexpr               auto            operator -  (void)               const   noexcept { return T_PARENT{ -m_Value }; }
            constexpr               auto            operator +  (const T_PARENT X)    const   noexcept { return T_PARENT{ m_Value + X.m_Value }; }
            constexpr               auto            operator -  (const T_PARENT X)    const   noexcept { return T_PARENT{ m_Value - X.m_Value }; }
            constexpr               auto            operator *  (const T_PARENT X)    const   noexcept { return T_PARENT{ m_Value * X.m_Value }; }
            constexpr               auto            operator /  (const T_PARENT X)    const   noexcept { return T_PARENT{ m_Value / X.m_Value }; }
        };


        struct degree : type<degree, float>
        {
            using type::type;
            using type::operator =;
            constexpr explicit                      degree(const unsigned long long    X)       noexcept : type{ static_cast<float>(X) } {}
            constexpr explicit                      degree(const double X)                      noexcept : type{ static_cast<float>(X) } {}
        };

        struct radian : type<radian, float>
        {
            using type::type;
            using type::operator =;
            constexpr explicit                      radian(const int X)                 noexcept : type{ static_cast<float>(X) } {}
            constexpr explicit                      radian(const long double X)         noexcept : type{ static_cast<float>(X) } {}
            constexpr                               radian(const degree X)              noexcept : type{ X.m_Value * 0.017453292519943295769f } {}
            constexpr degree                        getDegrees(void)                const   noexcept { return degree{ m_Value * 57.29577951308232087685f }; }
        };
        static float Sin(const radian X)
        { 
            return sinf(X.m_Value); 
        }

        glm::quat BlendAccurate(const glm::quat& Start, float T, const glm::quat& End) const
        {
            bool bFlip = false;

            // Determine if quats are further than 90 degrees
            float Cs = Dot(Start, End);
            if (Cs < 0.0f)
            {
                Cs = -Cs;
                bFlip = !bFlip;
            }

            float inv_T;
            if ((1.0f - Cs) < 0.000001f)
            {
                inv_T = 1.0f - T;
            }
            else
            {
                const radian Theta = radian{ acosf(Cs) };
                const float S = 1.0f / Sin(Theta);

                inv_T = Sin(radian{ 1.0f - T } * Theta)* S;
                T = Sin(radian{T} * Theta)* S;
            }

            if (bFlip)
            {
                T = -T;
            }

            return End * T + (Start) *inv_T;
        }

        void ComputeMatrices(std::span<glm::mat4> FinalL2W, const glm::mat4& L2W) const
        {
            auto& Anim = m_AnimPackage.m_Animations[m_iCurAnim];
            const float FrameTime = m_Time * Anim.m_FPS;
            const int   iFrameT0 = static_cast<int>(FrameTime);
            const int   iFrameT1 = static_cast<int>((iFrameT0 + 1) % Anim.m_BoneKeyFrames[0].m_Scale.size());

            // compute hierarchy matrices
            for (int i = 0; i < Anim.m_BoneKeyFrames.size(); ++i)
            {
                auto& KeyFrame = Anim.m_BoneKeyFrames[i];

                // Setup the L2W matrix for this bone, (two versions) one with inner-frame blending, the other version without it
                //FinalL2W[i].setup(xcore::math::transform3::Blend(KeyFrame.m_Transfoms[iFrameT0], FrameTime - iFrameT0, KeyFrame.m_Transfoms[iFrameT1]));

                glm::vec3 BlendedScale = KeyFrame.m_Scale[iFrameT0] + ((FrameTime - iFrameT0) * (KeyFrame.m_Scale[iFrameT1] - KeyFrame.m_Scale[iFrameT0]));
                glm::quat BlendedRotate = BlendAccurate(KeyFrame.m_Rotate[iFrameT0], (FrameTime - iFrameT0), KeyFrame.m_Rotate[iFrameT1]);
                glm::vec3 BlendedTranslate = KeyFrame.m_Translate[iFrameT0] + ((FrameTime - iFrameT0) * (KeyFrame.m_Translate[iFrameT1] - KeyFrame.m_Translate[iFrameT0]));

                //glm::mat4 Scale(1.f); glm::mat4 Translate(1.f);
                //Scale = glm::scale(Scale, BlendedScale);
                //glm::mat4 Rotate = setRotation(BlendedRotate);
                //Translate = glm::translate(Translate, BlendedTranslate);

                glm::mat4 Scale(1.f); glm::mat4 Translate(1.f);
                Scale = glm::scale(Scale, KeyFrame.m_Scale[iFrameT0]);
                glm::mat4 Rotate = glm::mat4_cast(KeyFrame.m_Rotate[iFrameT0]);
                Translate = glm::translate(Translate, KeyFrame.m_Translate[iFrameT0]);

                //FinalL2W[i] = Translate * Rotate * Scale;
                FinalL2W[i] = Scale * Rotate * Translate;

                if (-1 != m_Skeleton.m_Bones[i].m_iParent)
                {
                    assert(m_Skeleton.m_Bones[i].m_iParent <= i);
                    FinalL2W[i] = FinalL2W[m_Skeleton.m_Bones[i].m_iParent] * FinalL2W[i];
                }
                else
                {
                    assert(i == 0);
                    FinalL2W[i] = L2W * FinalL2W[i];
                }
            }

            // add the bind matrices into the final hierarchy
            for (int i = 0; i < Anim.m_BoneKeyFrames.size(); ++i)
            {
                FinalL2W[i] *= m_Skeleton.m_Bones[i].m_InvBind;
            }
        }

        const Skeleton& m_Skeleton;
        const anim_package& m_AnimPackage;
        int                 m_iCurAnim{};
        float               m_Time{};
    };

    struct AnimationGeom
    {
        geom            m_SkinGeom;
        Skeleton        m_Skeleton;
        anim_package    m_AnimPackage;
        anim_player     m_AnimPlayer{ m_Skeleton, m_AnimPackage };
    };

	class AnimationTest
	{
		public:
			AnimationTest(const std::shared_ptr<RenderPass>& TargetPass);
			~AnimationTest();

            void UpdateAnimations(AnimationUBO& UBO, glm::mat4 L2W);
            void UpdateMaterial(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index);
			void BindPipeline(VkCommandBuffer CmdBuffer);
            void BindBuffers(VkCommandBuffer CmdBuffer);
            void Draw(VkCommandBuffer CmdBuffer);

            VkPipelineLayout GetPipelineLayout();
            const VkDescriptorSet& GetDescriptorSet(uint32_t Index);

        private:
            void CreateIndexBuffer(const std::vector<int>& indices);
            void CreateVertexBuffer(const std::vector<vertex>& vertices);

		private:
			std::string m_ModelFilePath;
			std::unique_ptr<Pipeline> m_AnimationPipeline;
            std::unique_ptr<AnimationGeom> m_AnimationCharacter;

            std::unique_ptr<Buffer> m_VertexBuffer;
            std::unique_ptr<Buffer> m_IndexBuffer;
            uint32_t m_VertexCount = 0;
            uint32_t m_IndexCount = 0;

            std::unique_ptr<Material> m_MaterialInstace;
	};
}