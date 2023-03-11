#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
using namespace std;

int globalBID = 2;

struct node;

struct indexEntry {
	int key;
	node* child;
};
struct dataEntry {
	int key;
	int value;
};
struct node {
	bool isLeaf = true;
	int BID = 0;
	vector<int> keys;
	vector<indexEntry*> indexEntries;
	vector<dataEntry*> dataEntries;
	node* next = NULL; // pointer to connect next node for leaf nodes
	node* parent=NULL;
	node* leftChild=NULL;
};
class BTree {
public:
	node* root;
	int depth;
	int blockSize;
	int entryMax;
	vector<node*> leafNodes;
	int nodeSize;

	BTree(const char* fileName, int blockSize) {

		this->nodeSize = 0;
		FILE* fd = NULL;
		int data[3];  //fread로 값을 저장할 배열 변수  

		fd = fopen(fileName, "rb");
		if (!fd) {
			this->blockSize = blockSize;
			this->root = NULL;
			this->depth = 0;
			this->entryMax = (this->blockSize - 4) / 8;
			int rootBID = 1;
			this->nodeSize = 0;

			if (this->root == NULL) rootBID = 1;
			else rootBID = root->BID;

			int buff[3];
			buff[0] = this->blockSize; buff[1] = rootBID; buff[2] = this->depth;
			FILE* fp;
			fp = fopen(fileName, "wb");
			if (fp == NULL)
			{
				fprintf(stderr, "오류\n");
			}
			fwrite(buff, sizeof(int), 3, fp);  //이진파일 작성 

			fclose(fp);
		}
		else {
			fread(data, sizeof(int), 3, fd);//rb로 변경  
			this->blockSize = data[0];
			this->depth = data[2];
			int rootBID = data[1];
			int blockEntryNum = this->blockSize / 4;
			this->entryMax = (this->blockSize - 4) / 8;
			this->nodeSize = 0;
			fseek(fd, 12 + ((data[1] - 1) * this->blockSize), SEEK_SET);
			long ft1 = ftell(fd);
			fseek(fd, 0, SEEK_END);
			long ft2 = ftell(fd);
		if (ft1==ft2) {
			fclose(fd);
		}

		else {
			
			int* nodeInfo = new int[blockEntryNum];
			FILE* fp;
			fp = fopen(fileName, "rb");
			fseek(fp, 12 + ((rootBID - 1) * this->blockSize), SEEK_SET);
			fread(nodeInfo, sizeof(int), blockEntryNum, fp);

			// root is a leaf
			if (this->depth == 0) {
				node* newLeaf = new node;
				newLeaf->isLeaf = true;
				newLeaf->BID = rootBID;

				for (int i = 0; i < blockEntryNum - 2; i += 2) {
					if (nodeInfo[i] == 0) break;
					newLeaf->keys.push_back(nodeInfo[i]);
					dataEntry* newDataEntry = new dataEntry;
					newDataEntry->key = nodeInfo[i];
					newDataEntry->value = nodeInfo[i + 1];
					newLeaf->dataEntries.push_back(newDataEntry);
				}
				newLeaf->parent = NULL;
				newLeaf->leftChild = NULL;
				newLeaf->next = NULL;
				this->root = newLeaf;
				this->nodeSize = 1;
			}
			else {
				int depthLevel = this->depth;
				node* newNLeaf = new node;
				newNLeaf->isLeaf = false;
				newNLeaf->BID = rootBID;

				depthLevel--;
				newNLeaf->leftChild = getInternalNode(&depthLevel, nodeInfo[0], &fp, &newNLeaf);
				for (int i{ 1 }; i < blockEntryNum - 1; i += 2) {
					if (nodeInfo[i] == 0) break;
					depthLevel--;
					newNLeaf->keys.push_back(nodeInfo[i]);
					indexEntry* newIndexEntry = new indexEntry;
					newIndexEntry->key = nodeInfo[i];
					newIndexEntry->child = getInternalNode(&depthLevel, nodeInfo[i+1], &fp, &newNLeaf);
					newNLeaf->indexEntries.push_back(newIndexEntry);

				}
				newNLeaf->parent = NULL;
				newNLeaf->next = NULL;
				this->root = newNLeaf;
				this->nodeSize++;
				for (unsigned int i{ 1 }; i < this->leafNodes.size(); i++) {
					this->leafNodes[i - 1]->next = this->leafNodes[i];
				}
				this->leafNodes[this->leafNodes.size() - 1]->next = NULL;
			}
			fclose(fp);
			fclose(fd);
		}
		}
		
		

		
	}
	node* getLeafNode(int BID, FILE** fp, node** parent,int* depthLevel) {
		int blockEntryNum = this->blockSize / 4;
		int* nodeInfo = new int[blockEntryNum];
		fseek((*fp), 12 + ((BID - 1) * this->blockSize), SEEK_SET);
		fread(nodeInfo, sizeof(int), blockEntryNum, (*fp));

		node* newLeaf = new node;
		newLeaf->isLeaf = true;
		newLeaf->BID = BID;
		newLeaf->parent = (*parent);
		newLeaf->leftChild = NULL;
		for (int i{ 0 }; i < blockEntryNum - 2; i += 2) {
			if (nodeInfo[i] == 0) break;
			newLeaf->keys.push_back(nodeInfo[i]);
			dataEntry* newDataEntry = new dataEntry;
			newDataEntry->key = nodeInfo[i];
			newDataEntry->value = nodeInfo[i+1];
			newLeaf->dataEntries.push_back(newDataEntry);
		}
		this->leafNodes.push_back(newLeaf);
		this->nodeSize++;
		(*depthLevel) += 1;

		return newLeaf;
	}
	node* getInternalNode(int* depthLevel, int BID,FILE** fp,node** parent) {
		if ((*depthLevel) < 1) return getLeafNode(BID, &(*fp), &(*parent),&(*depthLevel));
		else {
			int blockEntryNum = this->blockSize / 4;
			int* nodeInfo = new int[blockEntryNum];
			fseek((*fp), 12 + ((BID - 1) * this->blockSize), SEEK_SET);
			fread(nodeInfo, sizeof(int), blockEntryNum, (*fp));

			(*depthLevel) -= 1;
			node* newNLeaf = new node;
			newNLeaf->isLeaf = false;
			newNLeaf->BID = BID;
			newNLeaf->parent = (*parent);
			newNLeaf->leftChild = getInternalNode(&(*depthLevel), nodeInfo[0], &(*fp), &newNLeaf);
			for (int i{ 1 }; i < blockEntryNum - 1; i += 2) {
				if (nodeInfo[i] == 0) break;
				(*depthLevel) -= 1;
				newNLeaf->keys.push_back(nodeInfo[i]);
				indexEntry* newIndexEntry = new indexEntry;
				newIndexEntry->key = nodeInfo[i];
				newIndexEntry->child = getInternalNode(&(*depthLevel), nodeInfo[i+1], &(*fp), &newNLeaf);
				newNLeaf->indexEntries.push_back(newIndexEntry);

			}
			newNLeaf->next = NULL;
			this->nodeSize++;
			(*depthLevel) += 1;
			return newNLeaf;
		}
	}
	void insert(int key, int value, int* rid) {
		dataEntry* newData = new dataEntry;
		newData->key = key;
		newData->value = value;
		if (this->root == NULL) {
			this->root = new node;
			root->isLeaf = true;
			root->parent = NULL;
			root->BID = 1;
			root->keys.push_back(key);
			root->dataEntries.push_back(newData);
			root->next = NULL;
			root->leftChild = NULL;
		}
		else {
			node* cursor = root;
			node* parent = NULL;
			// find position
			while (cursor->isLeaf == false) {
				parent = cursor;
				if (key < cursor->keys[0]) {
					cursor = cursor->leftChild;
				}
				else {
					int idx = std::upper_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
					cursor = cursor->indexEntries[idx-1]->child;
				}
			}
			// just insert
			int idx = upper_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
			cursor->keys.push_back(key);
			cursor->dataEntries.push_back(newData);

			if (idx != cursor->keys.size() - 1) {
				for (int i = cursor->keys.size() - 1; i > idx; i--) {
					cursor->keys[i] = cursor->keys[i - 1];
					cursor->dataEntries[i] = cursor->dataEntries[i - 1];
				}
				cursor->keys[idx] = key;
				cursor->dataEntries[idx] = newData;
			}
			// split from leaf node
			if ((int)cursor->keys.size() > this->entryMax) {
				indexEntry* newIndexEntry = new indexEntry;
				newIndexEntry->key = cursor->keys[this->entryMax / 2 + 1];
				node* newLeaf = new node;
				newIndexEntry->child = newLeaf;
				newLeaf->isLeaf = true;
				newLeaf->parent = cursor->parent;
				newLeaf->BID = *rid;
				*rid += 1;
				newLeaf->next = cursor->next;
				cursor->next = newLeaf;
				newLeaf->leftChild = NULL;
				for (int i = this->entryMax / 2 + 1 ; i <= this->entryMax ; i++) {
					newLeaf->keys.push_back(cursor->keys[i]);
					newLeaf->dataEntries.push_back(cursor->dataEntries[i]);
				}
				for (int i = this->entryMax / 2 + 1 ; i <= this->entryMax ; i++) {
					cursor->keys.pop_back();
					cursor->dataEntries.pop_back();
				}
				if (cursor == root) {
					node* newRoot = new node;
					newRoot->leftChild = cursor;
					newRoot->isLeaf = false;
					newRoot->BID = *rid;
					*rid += 1;
					newRoot->keys.push_back(newLeaf->keys[0]);
					newRoot->indexEntries.push_back(newIndexEntry);
					newRoot->next = NULL;
					newRoot->parent = NULL;
					cursor->parent = newRoot;
					newLeaf->parent = cursor->parent;
					this->depth++;
					this->root = newRoot;
					this->nodeSize++;
				}
				// not root, internal insert
				else {
					insertInternal(&newIndexEntry, &parent, &newLeaf,&(*rid));
					
				}
				this->nodeSize++;
			}
			
			
		}
		
	}
	void insertInternal(indexEntry** idxEntry, node** cursor, node** child,int* rid) {
		int key = (*child)->keys[0];
		int idx = upper_bound((*cursor)->keys.begin(), (*cursor)->keys.end(), key) - (*cursor)->keys.begin();
		(*cursor)->keys.push_back(key);
		(*cursor)->indexEntries.push_back((*idxEntry));

		if (idx != (*cursor)->keys.size() - 1) {
			for (int i = (*cursor)->keys.size() - 1; i > idx; i--) {
				(*cursor)->keys[i] = (*cursor)->keys[i - 1];
				(*cursor)->indexEntries[i] = (*cursor)->indexEntries[i - 1];
			}
			(*cursor)->keys[idx] = key;
			(*cursor)->indexEntries[idx] = (*idxEntry);
		}

		// split recursion
		if ((int)(*cursor)->keys.size() > this->entryMax) {
			indexEntry* newIndexEntry = new indexEntry;
			newIndexEntry->key = (*cursor)->keys[(this->entryMax + 1) / 2 ];
			node* newInternal = new node;
			newIndexEntry->child = newInternal;
			newInternal->isLeaf = false;
			newInternal->BID = *rid;
			*rid += 1;
			newInternal->next = NULL;
			newInternal->parent = (*cursor)->parent;
			newInternal->leftChild = (*cursor)->indexEntries[(this->entryMax + 1) / 2 ]->child;
			(*cursor)->indexEntries[(this->entryMax + 1) / 2]->child->parent = newInternal;
			for (int i{ (this->entryMax + 1) / 2 + 1 }; i <= this->entryMax ; i++) {
				newInternal->keys.push_back((*cursor)->keys[i]);
				newInternal->indexEntries.push_back((*cursor)->indexEntries[i]);
				(*cursor)->indexEntries[i]->child->parent = newInternal;
			}
			for (int i{ (this->entryMax + 1) / 2 }; i <= this->entryMax; i++) {
				(*cursor)->keys.pop_back();
				(*cursor)->indexEntries.pop_back();
			}
			if ((*cursor) == this->root) {
				node* newRoot = new node;
				newRoot->leftChild = (*cursor);
				newRoot->isLeaf = false;
				newRoot->BID =* rid;
				*rid += 1;
				newRoot->keys.push_back(newIndexEntry->key);
				newRoot->indexEntries.push_back(newIndexEntry);
				newRoot->next = NULL;
				newRoot->parent = NULL;
				(*cursor)->parent = newRoot;
				newInternal->parent = newRoot;
				this->depth++;
				this->root = newRoot;
				this->nodeSize++;
			}
			else {
				node* parent = (*cursor)->parent;
				insertInternal(&newIndexEntry, &parent, &newInternal, &(*rid));
			}
			this->nodeSize++;
		}
	}
	void print(const char* fileName) {
		if (this->root == NULL) cout << "error" << endl;
		else {
			char level[20] = "[level 0]";
			char level1[20] = "[level 1]";
			FILE* fw;
			fw = fopen(fileName, "w");
			if (fw == NULL)
			{
				fprintf(stderr, "오류\n");
			}
			node* cursor = root;
			if (cursor->isLeaf) {
				fprintf(fw, "%s\n", level);
				char what[30] = "Root is the leaf node";
				fprintf(fw, "%s\n", what);
			}
			// root is the internal node
			else {
				fprintf(fw, "%s\n", level);
				int keyToDisplay;
				for (int i{ 0 }; i < (int)cursor->indexEntries.size()-1; i++) {
					keyToDisplay = cursor->indexEntries[i]->key;
					fprintf(fw, "%d,", keyToDisplay);
				}
				keyToDisplay = cursor->indexEntries[cursor->indexEntries.size() - 1]->key;
				fprintf(fw, "%d\n\n", keyToDisplay);
				
				// recursion child write
				cursor = cursor->leftChild;
				fprintf(fw, "%s\n", level1);
				for (int i{ 0 }; i < (int)cursor->indexEntries.size(); i++) {
					keyToDisplay = cursor->indexEntries[i]->key;
					fprintf(fw, "%d,", keyToDisplay);
				}
				cursor = cursor->parent;
				for (int i{ 0 }; i < (int)cursor->indexEntries.size()-1; i++) {
					cursor = cursor->indexEntries[i]->child;
					if (cursor->isLeaf) {
						cout << "Here is leaf level." << endl;
					}
					else {
						for (int j{ 0 }; j < (int)cursor->indexEntries.size(); j++) {
							keyToDisplay = cursor->indexEntries[j]->key;
							fprintf(fw, "%d,", keyToDisplay);
						}
					}
					cursor = cursor->parent;
				}
				cursor = cursor->indexEntries[cursor->indexEntries.size() - 1]->child;
				for (int j{ 0 }; j < (int)cursor->indexEntries.size()-1; j++) {
					keyToDisplay = cursor->indexEntries[j]->key;
					fprintf(fw, "%d,", keyToDisplay);
				}
				keyToDisplay = cursor->indexEntries[cursor->indexEntries.size() - 1]->key;
				fprintf(fw, "%d", keyToDisplay);
			}
			fclose(fw);

		}
	}
	dataEntry* search(int key) // point search
	{
		if (this->root == NULL) return NULL;
		else {
			node* cursor = this->root;
			while (cursor->isLeaf == false) {
				int idx = upper_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
				if (key < cursor->keys[0]) cursor = cursor->leftChild;
				else cursor = cursor->indexEntries[idx - 1]->child;
			}
			int idx = lower_bound(cursor->keys.begin(), cursor->keys.end(), key) - cursor->keys.begin();
			if (idx == cursor->keys.size() || key != cursor->keys[idx]) return NULL;
			return cursor->dataEntries[idx];
		}
	}
	void rangeSearch(const char* fileName, const char* fileName2) // range search
	{
		if (this->root == NULL) cout << "error" << endl;
		else {
			int startRange, endRange;
			FILE* fr;
			fr = fopen(fileName2, "r");
			FILE* fp;
			fp = fopen(fileName, "w");
			char line[12], key[6], value[6];
			while (!feof(fr)) {
				if (fgets(line, 12, fr) == NULL) break;
				sscanf(line, "%[^,], %s", key, value);
				startRange = atoi(key);
				endRange = atoi(value);
				if (startRange == 0) break;

				// find position for startRange
				node* cursor1 = this->root;
				while (cursor1->isLeaf == false) {
					int idx = upper_bound(cursor1->keys.begin(), cursor1->keys.end(), startRange) - cursor1->keys.begin();
					if (startRange < cursor1->keys[0]) cursor1 = cursor1->leftChild;
					else cursor1 = cursor1->indexEntries[idx - 1]->child;
				}
				int idx1 = lower_bound(cursor1->keys.begin(), cursor1->keys.end(), startRange) - cursor1->keys.begin();
				if (idx1 == cursor1->keys.size()) {
					cursor1 = cursor1->next;
					idx1 = 0;
				}
				// find position for endRange
				node* cursor2 = this->root;
				
				if (cursor1 != NULL) {
					while (cursor2->isLeaf == false) {
						int idx = upper_bound(cursor2->keys.begin(), cursor2->keys.end(), endRange) - cursor2->keys.begin();
						if (endRange < cursor2->keys[0]) cursor2 = cursor2->leftChild;
						else cursor2 = cursor2->indexEntries[idx - 1]->child;
					}
					int idx2 = upper_bound(cursor2->keys.begin(), cursor2->keys.end(), endRange) - cursor2->keys.begin();
					idx2--;
					if (idx2 < 0) {
						idx2 = 0;
						if (cursor1 == cursor2 && idx1 > idx2) {
							cout << "error" << endl;
						}
						else {
							int buff[2];
							while (cursor1 != cursor2) {
								for (int i{ idx1 }; i < (int)cursor1->dataEntries.size(); i++) {
									buff[0] = cursor1->dataEntries[i]->key; buff[1] = cursor1->dataEntries[i]->value;
									fprintf(fp, "%d,%d / ", buff[0], buff[1]);
								}
								idx1 = 0;
								cursor1 = cursor1->next;
							}
						}
					}
					else {
						if (cursor1 == cursor2 && idx1 > idx2) {
							cout << "error" << endl;
						}
						else {
							int buff[2];
							while (cursor1 != cursor2) {
								for (int i{ idx1 }; i < (int)cursor1->dataEntries.size(); i++) {
									buff[0] = cursor1->dataEntries[i]->key; buff[1] = cursor1->dataEntries[i]->value;
									fprintf(fp, "%d,%d / ", buff[0], buff[1]);
								}
								idx1 = 0;
								cursor1 = cursor1->next;
							}
							for (int i{ idx1 }; i <= idx2; i++) {
								buff[0] = cursor1->dataEntries[i]->key; buff[1] = cursor1->dataEntries[i]->value;
								fprintf(fp, "%d,%d / ", buff[0], buff[1]);
							}
							fprintf(fp, "\n");
						}
					}
				}
				else {
					cout << "error" << endl;
				}
			}
			fclose(fp);
			fclose(fr);
		}
	}
	void fileWrite(FILE** fw) {

		int buff[3];
		buff[0] = this->blockSize; buff[1] = this->root->BID; buff[2] = this->depth;
		
		if ((*fw) == NULL)
		{
			fprintf(stderr, "오류\n");
		}
		fwrite(buff, sizeof(int), 3, (*fw));  //이진파일 작성 

		int tmp[2];
		int tmp2[1];
		int padding[1];
		padding[0] = 0;
		node* cursor = this->root;
		if (this->root == NULL);
		else {
			// root is the leaf
			if (cursor->isLeaf) {
				fseek((*fw), 12 + ((cursor->BID - 1) * this->blockSize), SEEK_SET);
				for (int i{ 0 }; i < (int)cursor->dataEntries.size() ; i++) {
					tmp[0] = cursor->dataEntries[i]->key;
					tmp[1] = cursor->dataEntries[i]->value;
					fwrite(tmp, sizeof(int), 2, (*fw));
				}
				for (int i = 0; i < this->blockSize / 4 - ((int)cursor->dataEntries.size()) * 2; i++) {
					fwrite(padding, sizeof(int), 1, (*fw));
				}
			}
			// root is the internal node
			else {
				// file write
				fseek((*fw), 12 + ((cursor->BID - 1) * this->blockSize), SEEK_SET);
				tmp2[0] = cursor->leftChild->BID;
				fwrite(tmp2, sizeof(int), 1, (*fw));
				for (int i{ 0 }; i < (int)cursor->indexEntries.size() ; i++) {
					tmp[0] = cursor->indexEntries[i]->key;
					tmp[1] = cursor->indexEntries[i]->child->BID;
					fwrite(tmp, sizeof(int), 2, (*fw));
				}
				for (int i = 0; i < this->blockSize / 4 - (int)(cursor->indexEntries.size()) * 2 - 1; i++) {
					fwrite(padding, sizeof(int), 1, (*fw));
				}
				// recursion child write
				cursor = cursor->leftChild;
				internalWrite(&(*fw), &cursor);
				for (int i{ 0 }; i < (int)cursor->indexEntries.size(); i++) {
					cursor = cursor->indexEntries[i]->child;
					internalWrite(&(*fw), &cursor);
				}
			}
		}


	}
	void internalWrite(FILE** fp, node** cursor) {
		if ((*cursor)->isLeaf) leafWrite(&(*fp), &(*cursor));
		else {
			int tmp2[1];
			int tmp[2];
			int padding[1];
			padding[0] = 0;

			fseek((*fp), 12 + (((*cursor)->BID - 1) * this->blockSize), SEEK_SET);
			tmp2[0] = (*cursor)->leftChild->BID;
			fwrite(tmp2, sizeof(int), 1, (*fp));

			for (int i{ 0 }; i < (int)(*cursor)->indexEntries.size(); i++) {
				tmp[0] = (*cursor)->indexEntries[i]->key;
				tmp[1] = (*cursor)->indexEntries[i]->child->BID;
				fwrite(tmp, sizeof(int), 2, (*fp));
			}

			for (int i = 0; i < this->blockSize / 4 - ((int)(*cursor)->indexEntries.size()) * 2 - 1; i++) {
				fwrite(padding, sizeof(int), 1, (*fp));
			}

			(*cursor) = (*cursor)->leftChild;
			internalWrite(&(*fp), &(*cursor));
			for (int i{ 0 }; i < (int)(*cursor)->indexEntries.size(); i++) {
				(*cursor) = (*cursor)->indexEntries[i]->child;
				internalWrite(&(*fp), &(*cursor));
			}
			(*cursor) = (*cursor)->parent;
		}
	}
	void leafWrite(FILE** fw, node** cursor) {
		int tmp2[1];
		int tmp[2];
		int padding[1];
		padding[0] = 0;

		fseek((*fw), 12 + (((*cursor)->BID - 1) * this->blockSize), SEEK_SET);
		for (int i{ 0 }; i < (int)(*cursor)->dataEntries.size(); i++) {
			tmp[0] = (*cursor)->dataEntries[i]->key;
			tmp[1] = (*cursor)->dataEntries[i]->value;
			fwrite(tmp, sizeof(int), 2, (*fw));
		}
		for (int i = 0; i < this->blockSize / 4 - ((int)(*cursor)->dataEntries.size()) * 2 - 1; i++) {
			fwrite(padding, sizeof(int), 1, (*fw));
		}
		if ((*cursor)->next == NULL) {
			fwrite(padding, sizeof(int), 1, (*fw));
		}
		else {
			tmp2[0] = (*cursor)->next->BID;
			fwrite(tmp2, sizeof(int), 1,(*fw));
		}
		(*cursor) = (*cursor)->parent;
	}
};
// Test
int main(int argc, char* argv[])
{
	char command = argv[1][0];
	
	int data[3];
	switch (command)
	{
	case 'c': {
		int block_size = atoi(argv[3]);
		BTree* yourBtree = new BTree(argv[2], block_size); }
		// create index file
		break;
	case 'i': {
		int rid;
		FILE* fd1 = NULL;
		//fread로 값을 저장할 배열 변수  
		int block_size2;
		fd1 = fopen(argv[2], "rb");  //rb로 변경  
		if (!fd1) return 0;//fread로 읽 기  
		else {
			fread(data, sizeof(int), 3, fd1);
			block_size2 = data[0];
		}
		
		BTree* yourBtree2 = new BTree(argv[2], block_size2);
		fseek(fd1, 0, SEEK_END);
		rid = ftell(fd1);
		rid = ((rid - 12) / block_size2) + 1;
		if (rid == 1) rid = 2;
		fclose(fd1);

		FILE* fp1;
		fp1 = fopen(argv[3], "r");
		char line[12], key[6], value[6];
		int num1, num2;
		while (!feof(fp1)) {
			if (fgets(line, 12, fp1) == NULL) break;
			sscanf(line, "%[^,], %s", key, value);
			num1 = atoi(key);
			num2 = atoi(value);
			yourBtree2->insert(num1, num2, &rid);
		}
		FILE* fw;
		fw = fopen(argv[2], "wb");

		yourBtree2->fileWrite(&fw);

		fclose(fw);
		fclose(fp1);
		cout << yourBtree2->root->keys[0];

	}
		// insert records from [records data file], ex) records.txt
		break;
	case 's': {
		FILE* fd2 = NULL;
		//fread로 값을 저장할 배열 변수  
		int block_size5;
		fd2 = fopen(argv[2], "rb");  //rb로 변경  
		if (!fd2) return 0;//fread로 읽 기  
		else {
			fread(data, sizeof(int), 3, fd2);
			block_size5 = data[0];
		}
		BTree* yourBtree3 = new BTree(argv[2], block_size5);
		fclose(fd2);

		FILE* fp2;
		fp2 = fopen(argv[3], "r");
		char line2[6];
		int searchKey;
		FILE* fw2;
		fw2 = fopen(argv[4], "w");
		while (!feof(fp2)) {
			if (fgets(line2, 6, fp2) == NULL) break;
			searchKey = atoi(line2);
			if (searchKey == 0) break;
			dataEntry* whatYouWant = yourBtree3->search(searchKey);
			fprintf(fw2, "%d,%d\n", whatYouWant->key, whatYouWant->value);
		}
		fclose(fw2);
		fclose(fp2); }
		// search keys in [input file] and print results to [output file]
		break;
	case 'r': {
		FILE* fd3 = NULL;
		//fread로 값을 저장할 배열 변수  
		int block_size7;
		fd3 = fopen(argv[2], "rb");  //rb로 변경  
		if ( !fd3) return 0;//fread로 읽 기  
		else {
			fread(data, sizeof(int), 3, fd3);
			block_size7 = data[0];
		}
		BTree* yourBtree4 = new BTree(argv[2], block_size7);
		fclose(fd3);

		yourBtree4->rangeSearch(argv[4], argv[3]);
	}
		// search keys in [input file] and print results to [output file]
		break;
	case 'p': {
		FILE* fd4 = NULL;
		//fread로 값을 저장할 배열 변수  
		int block_size6;
		fd4 = fopen(argv[2], "rb");  //rb로 변경  
		if (!fd4) return 0;//fread로 읽 기  
		else {
			fread(data, sizeof(int), 3, fd4);
			block_size6 = data[0];
		}
		BTree* yourBtree5 = new BTree(argv[2], block_size6);

		yourBtree5->print(argv[3]);
		fclose(fd4);
	}
		// print B+-Tree structure to [output file]
		break;
	}
};