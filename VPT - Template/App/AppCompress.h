#pragma once
#include "../processing.h"

class CAppCompress: public CProcessing
{
public:
    // Add variables here
    unsigned char *b;
    unsigned char *g;
    unsigned char *r;
    
    string encoded_sequence[3];
    
    string code_dict[256];
    struct Node {
        unsigned int freq;
        unsigned char ch;
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

    unsigned char *Compress(int &cDataSize) ;
    void Decompress(unsigned char *compressedData, int cDataSize, unsigned char *deCompressedData) ;

    void GetRGBArray();
    void getPrediction(unsigned char *channel, unsigned char *prediction);
    void getFilteredImage(unsigned char *filtered_b, unsigned char *filtered_g, unsigned char *filtered_r);
    void countIntensity(unsigned char *channel);
    Node* newNode(unsigned char data, unsigned freq);
    MinHeap* createMinHeap(unsigned size);
    void swapHeapNode(Node** a, Node** b);
    void heapify(MinHeap* heap, int idx);
    Node* extractMin(MinHeap* minHeap);
    void insertMinHeap(MinHeap* minHeap, Node* node);
    void buildMinHeap(MinHeap* minHeap);
    int CAppCompress::isLeaf(Node* root);
    MinHeap* createAndBuildMinHeap(char data[], int freq[], int size);
    int isSizeOne(struct MinHeap* minHeap);
    Node* buildHuffmanTree(char data[], int freq[], int size);
    void traverse(Node* node, string* code_dict, code);
    void HuffmanEncode(char data[], int freq[], int size);
    void HuffmanDecode(Node* root, string* encoded_sequence; unsigned char** data);
    void CAppCompress::DiffDecode();
    void HuffmanTree(int *hist);

public:
    void CustomInit(CView *pView) ;
    void Process(void) ;
    void CustomFinal(void) ;
};