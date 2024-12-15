/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: pgpBinaryTree.c,v 1.7.8.1 1999/06/15 22:36:19 build Exp $
____________________________________________________________________________*/

#include "pgpPFLErrors.h"

#include "pgpDebug.h"
#include "pgpBinaryTree.h"



typedef enum EBalance {
	balance_Left,
	balance_Equal,
	balance_Right
} EBalance;


typedef struct SPGPBinaryTreeNode {
	PGPInt32					key;
	PGPUserValue				userValue;
	struct SPGPBinaryTreeNode *	left;
	struct SPGPBinaryTreeNode *	right;
	EBalance					balance;
} SPGPBinaryTreeNode;


typedef struct SPGPBinaryTree {
	PGPMemoryMgrRef			memoryMgr;
	SPGPBinaryTreeNode *	firstNode;
} SPGPBinaryTree;



#define RotateRight_(x)	{									\
	SPGPBinaryTreeNode *	temp = (*(x))->left;			\
															\
	(*(x))->left = temp->right;								\
	temp->right = *(x);										\
	*(x) = temp;											\
}



#define RotateLeft_(x)	{									\
	SPGPBinaryTreeNode *	temp = (*(x))->right;			\
															\
	(*(x))->right = temp->left;								\
	temp->left = *(x);										\
	*(x) = temp;											\
}



	static PGPError
Insert(
	PGPMemoryMgrRef			memoryMgr,
	SPGPBinaryTreeNode **	root,
	PGPInt32				key,
	PGPUserValue			data,
	PGPBoolean *			taller)
{
	PGPError				result = kPGPError_NoErr;
	PGPBoolean				tallerSub;
	SPGPBinaryTreeNode *	left;
	SPGPBinaryTreeNode *	right;
	
	if (*root == NULL) {
		if ((*root = (SPGPBinaryTreeNode *) PGPNewData(memoryMgr,
		sizeof(SPGPBinaryTreeNode), 0)) == NULL) {
			*taller = FALSE;
			result = kPGPError_OutOfMemory;
		} else {
			(*root)->left = NULL;
			(*root)->right = NULL;
			(*root)->balance = balance_Equal;
			(*root)->key = key;
			(*root)->userValue = data;
			*taller = TRUE;
		}
	} else if (key == (*root)->key) {
		*taller = FALSE;
		result = kPGPError_ItemAlreadyExists;
	} else if (key > (*root)->key) {
		result = Insert(memoryMgr, &((*root)->right), key, data, &tallerSub);
		if (tallerSub) {
			switch ((*root)->balance) {
				case balance_Left:
				{
					(*root)->balance = balance_Equal;
					*taller = FALSE;
				}
				break;
				
				
				case balance_Equal:
				{
					(*root)->balance = balance_Right;
					*taller = TRUE;
				}
				break;
				
				
				case balance_Right:
				{
					right = (*root)->right;
					switch (right->balance) {
						case balance_Left:
						{
							left = right->left;
							switch (left->balance) {
								case balance_Left:
								{
									(*root)->balance = balance_Equal;
									right->balance = balance_Right;
								}
								break;
								
								
								case balance_Equal:
								{
									(*root)->balance = balance_Equal;
									right->balance = balance_Equal;
								}
								break;
								
								
								case balance_Right:
								{
									(*root)->balance = balance_Left;
									right->balance = balance_Equal;
								}
								break;
							}
							left->balance = balance_Equal;
							RotateRight_(&right);
							(*root)->right = right;
							RotateLeft_(root);
							*taller = FALSE;
						}
						break;
						
						
						case balance_Right:
						{
							(*root)->balance = balance_Equal;
							right->balance = balance_Equal;
							RotateLeft_(root);
							*taller = FALSE;
						}
						break;
						
						case balance_Equal:
							/* Needed to silence compiler */
							break;
					}
				}
				break;
			}
		} else {
			*taller = FALSE;
		}
	} else {
		result = Insert(memoryMgr, &((*root)->left), key, data, &tallerSub);
		if (tallerSub) {
			switch ((*root)->balance) {
				case balance_Left:
				{
					left = (*root)->left;
					switch (left->balance) {
						case balance_Left:
						{
							(*root)->balance = balance_Equal;
							left->balance = balance_Equal;
							RotateRight_(root);
							*taller = FALSE;
						}
						break;
						
						
						case balance_Right:
						{
							right = left->right;
							switch (right->balance) {
								case balance_Left:
								{
									(*root)->balance = balance_Right;
									left->balance = balance_Equal;
								}
								break;
								
								
								case balance_Equal:
								{
									(*root)->balance = balance_Equal;
									left->balance = balance_Equal;
								}
								break;
								
								
								case balance_Right:
								{
									(*root)->balance = balance_Equal;
									left->balance = balance_Left;
								}
								break;
							}
							right->balance = balance_Equal;
							RotateLeft_(&left);
							(*root)->left = left;
							RotateRight_(root);
							*taller = FALSE;
						}
						break;

						case balance_Equal:
							/* Needed to silence compiler */
							break;
					}
				}
				break;
				
				
				case balance_Equal:
				{
					(*root)->balance = balance_Left;
					*taller = TRUE;
				}
				break;
				
				
				case balance_Right:
				{
					(*root)->balance = balance_Equal;
					*taller = FALSE;
				}
				break;
			}
		} else {
			*taller = FALSE;
		}
	}
	
	return result;
}



	PGPError
PGPAddNode(
	PGPBinaryTreeRef	inBinaryTree,
	PGPInt32			inKey,
	PGPUserValue		inUserValue)
{
	PGPBoolean	taller;
	
	return Insert(inBinaryTree->memoryMgr, &inBinaryTree->firstNode,
				inKey, inUserValue, &taller);
}



	static PGPBoolean
RightBalance(
	SPGPBinaryTreeNode **	root)
{
	PGPBoolean				result = FALSE;
	SPGPBinaryTreeNode *	left;
	SPGPBinaryTreeNode *	right;

	switch ((*root)->balance) {
		case balance_Right:
		{
			(*root)->balance = balance_Equal;
			result = TRUE;
		}
		break;
		
		
		case balance_Equal:
		{
			(*root)->balance = balance_Left;
			result = FALSE;
		}
		break;
		
		
		case balance_Left:
		{
			left = (*root)->left;
			switch (left->balance) {
				case balance_Right:
				{
					right = left->right;
					switch (right->balance) {
						case balance_Right:
						{
							(*root)->balance = balance_Equal;
							left->balance = balance_Left;
						}
						break;
						
						
						case balance_Equal:
						{
							(*root)->balance = balance_Equal;
							left->balance = balance_Equal;
						}
						break;
						
						
						case balance_Left:
						{
							(*root)->balance = balance_Right;
							left->balance = balance_Equal;
						}
						break;
					}
					right->balance = balance_Equal;
					RotateLeft_(&left);
					(*root)->left = left;
					RotateRight_(root);
					result = TRUE;
				}
				break;
				
				
				case balance_Equal:
				{
					(*root)->balance = balance_Left;
					left->balance = balance_Right;
					RotateRight_(root);
					result = FALSE;
				}
				break;
				
				
				case balance_Left:
				{
					(*root)->balance = balance_Equal;
					left->balance = balance_Equal;
					RotateRight_(root);
					result = TRUE;
				}
				break;
			}
		}
		break;
	}
	
	return result;
}



	static PGPBoolean
LeftBalance(
	SPGPBinaryTreeNode **	root)
{
	PGPBoolean				result = FALSE;
	SPGPBinaryTreeNode *	left;
	SPGPBinaryTreeNode *	right;

	switch ((*root)->balance) {
		case balance_Left:
		{
			(*root)->balance = balance_Equal;
			result = TRUE;
		}
		break;
		
		
		case balance_Equal:
		{
			(*root)->balance = balance_Right;
			result = FALSE;
		}
		break;
		
		
		case balance_Right:
		{
			right = (*root)->right;
			switch (right->balance) {
				case balance_Left:
				{
					left = right->left;
					switch (left->balance) {
						case balance_Left:
						{
							(*root)->balance = balance_Equal;
							right->balance = balance_Right;
						}
						break;
						
						
						case balance_Equal:
						{
							(*root)->balance = balance_Equal;
							right->balance = balance_Equal;
						}
						break;
						
						
						case balance_Right:
						{
							(*root)->balance = balance_Left;
							right->balance = balance_Equal;
						}
						break;
					}
					left->balance = balance_Equal;
					RotateRight_(&right);
					(*root)->right = right;
					RotateLeft_(root);
					result = TRUE;
				}
				break;
				
				
				case balance_Equal:
				{
					(*root)->balance = balance_Right;
					right->balance = balance_Left;
					RotateLeft_(root);
					result = FALSE;
				}
				break;
				
				
				case balance_Right:
				{
					(*root)->balance = balance_Equal;
					right->balance = balance_Equal;
					RotateLeft_(root);
					result = TRUE;
				}
				break;
			}
		}
		break;
	}
	
	return result;
}



	static PGPUserValue
Delete(
	SPGPBinaryTreeNode **	root,
	PGPInt32				key,
	PGPBoolean *			shorter)
{
	PGPBoolean				shorterSubtree = FALSE;
	PGPUserValue			result;
	PGPInt32				resultKey;
	SPGPBinaryTreeNode *	minNode;
	SPGPBinaryTreeNode *	saveNode;
	
	if (*root == NULL) {
		*shorter = FALSE;
		result = NULL;
	} else if (key == (*root)->key) {
		saveNode = *root;
		result = saveNode->userValue;
		resultKey = saveNode->key;
		if ((*root)->left == NULL) {
			*root = (*root)->right;
			*shorter = TRUE;
			PGPFreeData(saveNode);
		} else if ((*root)->right == NULL) {
			*root = (*root)->left;
			*shorter = TRUE;
			PGPFreeData(saveNode);
		} else {
			minNode = (*root)->right;
			while (minNode->left != NULL) {
				minNode = minNode->left;
			}
			(*root)->key = minNode->key;
			(*root)->userValue = minNode->userValue;
			minNode->userValue = result;
			minNode->key = resultKey;
			result = Delete(&(*root)->right, key, &shorterSubtree);
			if (shorterSubtree) {
				*shorter = RightBalance(root);
			} else {
				*shorter = FALSE;
			}
		}
	} else if (key < (*root)->key) {
		if ((result = Delete(&(*root)->left, key, &shorterSubtree)) ==
		NULL) {
			*shorter = FALSE;
			result = NULL;
		} else if (shorterSubtree) {
			*shorter = LeftBalance(root);
		} else {
			*shorter = FALSE;
		}
	} else {
		if ((result = Delete(&(*root)->right, key, &shorterSubtree)) ==
		NULL) {
			*shorter = FALSE;
			result = NULL;
		} else if (shorterSubtree) {
			*shorter = RightBalance(root);
		} else {
			*shorter = FALSE;
		}
	}
	
	return result;
}



	PGPError
PGPDisposeNode(
	PGPBinaryTreeRef	inBinaryTree,
	PGPInt32			inKey,
	PGPUserValue *		outUserValue)
{
	PGPError	result = kPGPError_NoErr;
	PGPBoolean	shorter;
	
	*outUserValue = Delete(&inBinaryTree->firstNode, inKey, &shorter);
	if (*outUserValue == NULL) {
		result = kPGPError_ItemNotFound;
	}
	
	return result;
}



	PGPError
PGPNewBinaryTree(
	PGPMemoryMgrRef		inMemoryMgr,
	PGPBinaryTreeRef *	outBinaryTree)
{
	PGPError	result = kPGPError_NoErr;
	
	*outBinaryTree = PGPNewData(inMemoryMgr,sizeof(SPGPBinaryTree), 0);
	if (*outBinaryTree == NULL) {
		result = kPGPError_OutOfMemory;
	} else {
		(*outBinaryTree)->memoryMgr = inMemoryMgr;
		(*outBinaryTree)->firstNode = NULL;
	}
	
	return result;
}



	static PGPUInt32
FreeTree(
	SPGPBinaryTreeNode *	root)
{
	PGPUInt32	numLeft = 0;
	PGPUInt32	numRight = 0;
	PGPUInt32	result = 0;
	
	if (root != NULL) {
		if (root->left != NULL) {
			numLeft = FreeTree(root->left);
		}
		if (root->right != NULL) {
			numRight = FreeTree(root->right);
		}
		PGPFreeData(root);
		result = numLeft + numRight + 1;
	}
	
	return result;
}


					
	PGPError
PGPDisposeBinaryTree(
	PGPBinaryTreeRef	inBinaryTree)
{
	FreeTree(inBinaryTree->firstNode);
	PGPFreeData(inBinaryTree);

	return kPGPError_NoErr;
}



	static SPGPBinaryTreeNode *
Find(
	SPGPBinaryTreeNode *	root,
	PGPInt32				inKey)
{
	while ((root != NULL) && (inKey != root->key)) {
		if (inKey < root->key) {
			root = root->left;
		} else {
			root = root->right;
		}
	}
	
	return root;
}



	PGPError
PGPFindNode(
	PGPBinaryTreeRef	inBinaryTree,
	PGPInt32			inKey,
	PGPUserValue *		outUserValue)
{
	PGPError				result = kPGPError_NoErr;
	SPGPBinaryTreeNode *	node;
	
	node = Find(inBinaryTree->firstNode, inKey);
	if (node == NULL) {
		result = kPGPError_ItemNotFound;
	} else {
		*outUserValue = node->userValue;
	}
	
	return result;
}



	PGPError
PGPChangeNode(
	PGPBinaryTreeRef	inBinaryTree,
	PGPInt32			inKey,
	PGPUserValue		inNewUserValue,
	PGPUserValue *		outOldUserValue)
{
	PGPError				result = kPGPError_NoErr;
	SPGPBinaryTreeNode *	node;
	
	node = Find(inBinaryTree->firstNode, inKey);
	if (node == NULL) {
		result = kPGPError_ItemNotFound;
	} else {
		*outOldUserValue = node->userValue;
		node->userValue = inNewUserValue;
	}
	
	return result;
}



	static PGPError
InOrderTraversal(
	SPGPBinaryTreeNode *		root,
	PGPBinaryTreeTraversalProc	userProc,
	PGPUserValue				userValue)
{
	PGPError	result = kPGPError_NoErr;
	
	if (root != NULL) {
		result = InOrderTraversal(root->left, userProc, userValue);
		if (result == kPGPError_NoErr) {
			result = (*userProc)(userValue, root->key, root->userValue);
		}
		if (result == kPGPError_NoErr) {
			result = InOrderTraversal(root->right, userProc, userValue);
		}
	}
	
	return result;
}


	PGPError
PGPInOrderTraversal(
	PGPBinaryTreeRef			inBinaryTree,
	PGPBinaryTreeTraversalProc	inUserProc,
	PGPUserValue				inUserValue)
{
	return InOrderTraversal(inBinaryTree->firstNode, inUserProc, inUserValue);
}
