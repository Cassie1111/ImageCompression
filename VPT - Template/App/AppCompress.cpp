#include "StdAfx.h"
#include "AppCompress.h"

CAppCompress::CAppCompress(void)
{
    // Class Constructor
}

CAppCompress::~CAppCompress(void)
{
    // Class Destructor
    // Must call Final() function in the base class

    Final() ;
}

void CAppCompress::CustomInit(CView *pView) {
 // Add custom initialization code here
 // This initialization code will be called when this application is added to a processing task lists
}

void CAppCompress::CustomFinal(void) {
 // Add custom finalization code here
}

int predictEval(unsigned char *buf, int x, int y, int width, int height, int &diffValue) {

    int predT ;
    int predL ;
    int predTL ;
    int pred ;
    int left ;
    int top ;
    int topLeft ;
    int mode ;
    int actual ;
    int diff ;

    if(x <= 0) {
        left = 0 ;
        topLeft = 0 ;
    }
    if(y <= 0) {
        top = 0 ;
        topLeft = 0 ;
    }
    if(y > 0) {
        top = buf[x + (y - 1) * width] ;
    }
    if(x > 0 && y >= 0) {
        left = buf[(x - 1) + y * width] ;
    }
    if(x > 0 && y > 0) {
        topLeft = buf[(x - 1) + (y - 1) * width] ;
    }

    predT = top ;
    predL = left ;
    predTL = topLeft ;
    actual = buf[x + y * width] ;

    if(predL <= predT && predL <= predTL) {
        mode = 1 ;
        pred = predL ;
    } else if(predT <= predL && predT <= predTL) {
        mode = 2 ;
        pred = predT ;
    } else if(predTL <= predL && predTL <= predT) {
        mode = 3 ;
        pred = predTL ;
    }
    diff = actual - pred ;
    diff = diff >= 0 ? diff : -diff ;
    if(diff >= 8) {
        mode = 4 ;
        diffValue = actual ;
    } else {
        diffValue = actual - pred ;
    }
    
    return mode ;
}

unsigned char predDiff(unsigned char *buf, int x, int y, int width, int height, int mode, int diffValue) {

    int predT ;
    int predL ;
    int predTL ;
    int pred ;
    int left ;
    int top ;
    int topLeft ;

    if(x <= 0) {
      left = 0 ;
      topLeft = 0 ;
    }
    if(y <= 0) {
        top = 0 ;
        topLeft = 0 ;
    }
    if(y > 0) {
        top = buf[x + (y - 1) * width] ;
    }
    if(x > 0 && y >= 0) {
        left = buf[(x - 1) + y * width] ;
    }
    if(x > 0 && y > 0) {
        topLeft = buf[(x - 1) + (y - 1) * width] ;
    }
    predT = top ;
    predL = left ;
    predTL = topLeft ;

    switch(mode) {
        case 1:
            pred = predL + diffValue;
            break ;
        case 2:
            pred = predT + diffValue;
            break ;
        case 3:
            pred = predTL + diffValue;
            break ;
        case 4:
            pred = diffValue ;
            break ;
    }

    return (unsigned char) pred ;
}

void CAppCompress::getPrediction(unsigned char *channel, unsigned char *prediction) {
    int i, j;

    prediction[0] = channel[0];

    for (i = 1; i < width; i++) {
        prediction[i] = channel[i - 1];
    }

    for (j = 1; j < height; j++) {
        prediction[j * width] = channel[(j - 1) * width];
    }

    for (j = 1; j < height; j++) {
        for (i = 1; i < width; i++) {
            prediction[i + j * width] = (channel[(i - 1) + j * width] + channel[i + (j - 1) * width]) / 2;
        }
    }
}

void CAppCompress::getFilteredImage(unsigned char *filtered_b, unsigned char *filtered_g, unsigned char *filtered_r) {
    int dataSize = width * height;
    int i, j;

    unsigned char *prediction_b = new unsigned char[dataSize];
    unsigned char *prediction_g = new unsigned char[dataSize];
    unsigned char *prediction_r = new unsigned char[dataSize];

    getPrediction(this->b, prediction_b);
    getPrediction(this->g, prediction_g);
    getPrediction(this->r, prediction_r);

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            filtered_b[i + j * width] = b[i + j * width] - prediction_b[i + j * width];
            filtered_g[i + j * width] = g[i + j * width] - prediction_g[i + j * width];
            filtered_r[i + j * width] = r[i + j * width] - prediction_r[i + j * width];
        }
    }
}

void CAppCompress::countIntensity(unsigned char *channel) {
    int i, j;
    int hist[256];

    for (i = 0; i < 256; i++)
        hist[i] = 0;

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            hist[channel[i + j * width]] += 1;
        }
    }
}

CAppCompress::Node* CAppCompress:: newNode(unsigned char data, unsigned freq) {
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->left = NULL;
    temp->right = NULL;
    temp->ch = data;
    temp->freq = freq;

    return temp;
}

CAppCompress::MinHeap* CAppCompress::createMinHeap(unsigned size) {
    MinHeap* heap = (CAppCompress::MinHeap*)malloc(sizeof(MinHeap));

    heap->size = size;
    heap->array = (Node**)malloc(heap->size * sizeof(Node*));

    return heap;
}

void CAppCompress::swapHeapNode(Node** a, Node** b) {
    Node* t = *a;
    *a = *b;
    *b = t;
}

void CAppCompress::heapify(MinHeap* heap, int idx) {
    int min_idx = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size && heap->array[left]->freq < heap->array[min_idx]->freq) {
        min_idx = left;
    }

    if (right < heap->size && heap->array[right]->freq < heap->array[min_idx]->freq) {
        min_idx = right;
    }

    if (min_idx != idx) {
        swapHeapNode(&heap->array[min_idx], &heap->array[idx]);

        heapify(heap, min_idx);
    }
}

CAppCompress::Node* CAppCompress::extractMin(MinHeap* minHeap) {
    Node* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];

    minHeap->size -= 1;
    heapify(minHeap, 0);
}

void CAppCompress::insertMinHeap(MinHeap* minHeap, Node* node) {
    minHeap->size += 1;

    int i = minHeap->size - 1;
    int parent_idx = (i - 1) / 2;

    // insert the node to min heap
    while (i && node->freq < minHeap->array[parent_idx]->freq) {
        minHeap->array[i] = minHeap->array[parent_idx];
        i = parent_idx;
        parent_idx = (i - 1) / 2;
    }

    minHeap->array[i] = node;
}

void CAppCompress::buildMinHeap(MinHeap* minHeap) {
    int n = minHeap->size - 1;
    int i;

    for (i = (n - 1) / 2; i >= 0; --i) {
        heapify(minHeap, i);
    }
}

int CAppCompress::isLeaf(CAppCompress::Node* root) { 
  
    return !(root->left) && !(root->right); 
}

CAppCompress::MinHeap* createAndBuildMinHeap(char data[], int freq[], int size) { 
  
    MinHeap* minHeap = createMinHeap(size); 
  
    for (int i = 0; i < size; ++i) 
        minHeap->array[i] = newNode(data[i], freq[i]); 
  
    minHeap->size = size; 
    buildMinHeap(minHeap); 
  
    return minHeap; 
}

int CAppCompress::isSizeOne(struct MinHeap* minHeap) { 
  
    return (minHeap->size == 1); 
}

CAppCompress::Node* buildHuffmanTree(char data[], int freq[], int size) { 
    Node *left, *right, *top; 
  
    // Step 1: Create a min heap of capacity 
    // equal to size. Initially, there are 
    // modes equal to size. 
    MinHeap* minHeap = createAndBuildMinHeap(data, freq, size); 
  
    // Iterate while size of heap doesn't become 1 
    while (!isSizeOne(minHeap)) { 
  
        // Step 2: Extract the two minimum 
        // freq items from min heap 
        left = extractMin(minHeap); 
        right = extractMin(minHeap); 
  
        // Step 3: Create a new internal 
        // node with frequency equal to the 
        // sum of the two nodes frequencies. 
        // Make the two extracted node as 
        // left and right children of this new node. 
        // Add this node to the min heap 
        // '$' is a special value for internal nodes, not used 
        top = newNode('$', left->freq + right->freq); 
  
        top->left = left; 
        top->right = right; 
  
        insertMinHeap(minHeap, top); 
    } 
  
    // Step 4: The remaining node is the 
    // root node and the tree is complete. 
    return extractMin(minHeap); 
}

void CAppCompress::traverse(CAppCompress::Node* node, string* code_dict, code)
{
    if (node->left == NULL && node->right == NULL)
    {
        code_dict[node->ch] = code;
    }
    else
    {
        traverse(node->left, code_dict, code + '0');
        traverse(node->right, code_dict, code + '1');
    }
}

// Huffman Encoding using the Huffman tree built above
void CAppCompress::HuffmanEncode(char data[], int freq[], int size) {
    int i, j;
    Node* root = buildHuffmanTree(data, freq, size); 
  
    // the encoding string
    
    encoded_sequence[0] = encoded_sequence[1] = encoded_sequence[2] = "";
    
    // the coding dictionary
    for (i = 0; i < 256; i++)
        code_dict[i] = '';
    
    
   
    traverse(root, code_dict, "");
    
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            encoded_sequence[0] += code_dict[b[i + j * width]];
            encoded_sequence[1] += code_dict[b[i + j * width]];
            encoded_sequence[2] += code_dict[b[i + j * width]];
        }
    }   
}


void CAppCompress::HuffmanDecode(CAppCompress::Node* root, string* encoded_sequence; unsigned char** data) {
    int i, j;
    Node *temp = root;
    
    for (i = 0; i < 3; i++) {
        for (i = 0; i < encoded_sequence[i].size(); i++) {
            if (isLeaf(temp)) {
                data[i].append(temp->ch);
                temp = root;
                
            } else {
                if (encoded_sequence[i][j] == '0') {
                    temp = temp->left;
                } else (encoded_sequence[i][j] == '1') {
                    temp = temp->right;
                }
            } 
        }
    }
}

void CAppCompress::DiffDecode() {

}



void CAppCompress::HuffmanTree(int *hist) {
    Node *root = {};

    int i, j;
    int nodes_number = 0;
    int min = 256;
    int min_index = 0;

    for (i = 0; i < 256; i++) {
        if (hist[i] != 0) {
            nodes_number += 0;
        }
    }

    for (i = 0; i < nodes_number; i++) {
        for (j = 0; j < 256; j++) {
            if (hist[j] != 0 && hist[j] < min) {
                min = hist[j];
                min_index = j;
            }
        }
    }
}


// This function compresses input 24-bit image (8-8-8 format, in pInput pointer).
// This function shall allocate storage space for compressedData, and return it as a pointer.
// The input reference variable cDataSize, is also serve as an output variable to indicate the size (in bytes) of the compressed data.
unsigned char *CAppCompress::Compress(int &cDataSize) {

    // You can modify anything within this function, but you cannot change the function prototype.
    unsigned char *compressedData ;

    cDataSize = width * height * 3 ;    // You need to determine the size of the compressed data. 
             // Here, we simply set it to the size of the original image
    compressedData = new unsigned char[cDataSize] ; // As an example, we just copy the original data as compressedData.

    memcpy(compressedData, pInput, cDataSize) ;

    return compressedData ;  // return the compressed data
}

// This function takes in compressedData with size cDatasize, and decompresses it into 8-8-8 image.
// The decompressed image data should be stored into the uncompressedData buffer, with 8-8-8 image format
void CAppCompress::Decompress(unsigned char *compressedData, int cDataSize, unsigned char *uncompressedData) {

    // You can modify anything within this function, but you cannot change the function prototype.
    memcpy(uncompressedData, compressedData, cDataSize) ; // Here, we simply copy the compressedData into the output buffer.
}


void CAppCompress::Process(void) {

    // Don't change anything within this function.

    int i, cDataSize ;

    unsigned char *compressedData ;
    unsigned char *verifyCompressedData ;

    SetTitle(pOutput, _T("Lossless Decompressed Image")) ;

    compressedData = Compress(cDataSize) ;

    verifyCompressedData = new unsigned char [cDataSize] ;

    memcpy(verifyCompressedData, compressedData, cDataSize) ;

    delete [] compressedData ;

    Decompress(verifyCompressedData, cDataSize, pOutput) ;

    for(i = 0; i < width * height * 3; i++) {
        if(pInput[i] != pOutput[i]) {
            printf(_T("Caution: Decoded Image is not identical to the Original Image!\r\n")) ;
            break ;
        }
    }

    printf(_T("Original Size = %d, Compressed Size = %d, Compression Ratio = %2.2f\r\n"), width * height * 3, cDataSize, (double) width * height * 3 / cDataSize) ;

    PutDC(pOutput) ;
}