#pragma once
#include "Animation.hpp"

#define MAX_BONE_COUNT 100

namespace FUSIONOPENGL
{
	class Animator
	{
	public: 
		Animator(Animation* Animation);
		void UpdateAnimation(float dt);
		void UpdateBlendedAnimation(Animation* BaseAnimation, Animation* LayeredAnimation, float blendFactor, float deltaTime);
		void CalculateBlendedBoneTransform(Animation* pAnimationBase, const BoneNodeData* node, Animation* pAnimationLayer, const BoneNodeData* nodeLayered,
			const float currentTimeBase, const float currentTimeLayered, const glm::mat4& parentTransform, const float blendFactor);
		void PlayAnimation(Animation* Animation);
		void CalculateBoneTransform(const BoneNodeData* node, glm::mat4 parentTransform);
		void CalculateBoneTransform(Animation* animation , const BoneNodeData* node, const float currentTime, glm::mat4 parentTransform);
		std::vector<glm::mat4> GetFinalBoneMatrices();
	private:
		std::vector<glm::mat4> FinalBoneMatrices;
		std::vector<glm::mat4> GlobalBoneMatrices;
		Animation* CurrentAnimation;
		float CurrentTime;
		float DeltaTime;
	};

}