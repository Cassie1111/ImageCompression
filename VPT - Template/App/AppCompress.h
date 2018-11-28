#pragma once
#include "../processing.h"
#include <string>

using namespace std;

class CAppCompress : public CProcessing
{
public:
    // Add variables here
    unsigned char *b;
    unsigned char *g;
    unsigned char *r;

    string encoded_sequence[3];

    // for 3 channel, different value range[-255, 255]
    string code_dict[3][511];

    struct Node {
        unsigned int freq;
        int diff;
        Node *left, *right;
    };

    struct MinHeap {
        // current size of the heap;
        unsigned size;

        Node** array;
    };

public:
    CAppCompress(void);
    ~CAppCompress(void);
    // Add methods here

    unsigned char *Compress(int &cDataSize);
    void Decompress(unsigned char *compressedData, int cDataSize, unsigned char *deCompressedData);

    void GetRGBArray();
    void getPrediction(unsigned char *channel, unsigned char *prediction);
    void getFilteredImage(int *filtered_b, int *filtered_g, int *filtered_r);
    void countDiffIntensity(int *filtered_b, int *filtered_g, int *filtered_r, int *diff_b, int *diff_g, int *diff_r);
    int getDiffCount(int *diff);
    void splitDiffAndFreq(int *diff, int *data, int* freq);
    Node* newNode(int data, unsigned freq);
    MinHeap* createMinHeap(unsigned size);
    void swapHeapNode(Node** a, Node** b);
    void heapify(MinHeap* heap, int idx);
    Node* extractMin(MinHeap* minHeap);
    void insertMinHeap(MinHeap* minHeap, Node* node);
    void buildMinHeap(MinHeap* minHeap);
    int CAppCompress::isLeaf(Node* root);
    MinHeap* createAndBuildMinHeap(int data[], int freq[], int size);
    int isSizeOne(struct MinHeap* minHeap);
    Node* buildHuffmanTree(int data[], int freq[], int size);
    void traverse(Node* node, string* code_dict, string code);
    void HuffmanEncode();
    void convertDecodedStringToBytes(string encoded_sequence, unsigned char *encoded_data);
    void HuffmanDecode(Node* root, string* encoded_sequence, int** data);
    void CAppCompress::DiffDecode();
    void HuffmanTree(int *hist);

public:
    void CustomInit(CView *pView);
    void Process(void);
    void CustomFinal(void);
};