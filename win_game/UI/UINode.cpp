#include "UINode.h"
#include "UIImage.h"

bool UINode::CompareScenes(UINode* root1, UINode* root2)
{
	if (root1 == root2)
		return true;

	if (root1->AlignH != root2->AlignH ||
		root1->AlignV != root2->AlignV ||
		root1->Type != root2->Type ||
		root1->Offset != root2->Offset ||
		root1->Pivot != root2->Pivot ||
		root1->Size != root2->Size ||
		root1->Children.GetLen() != root2->Children.GetLen())
		return false;
	for (uint32 i = 0; i < root1->Children.GetLen(); ++i)
	{
		if (!CompareScenes(root1->Children[i], root2->Children[i]))
			return false;
	}
	return true;
}
