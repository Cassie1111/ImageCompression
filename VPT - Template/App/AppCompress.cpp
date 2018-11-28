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

    Final();
}

void CAppCompress::CustomInit(CView *pView) {
    // Add custom initialization code here
    // This initialization code will be called when this application is added to a processing task lists
}

void CAppCompress::CustomFinal(void) {
    // Add custom finalization code here
}

int predictEval(unsigned char *buf, int x, int y, int width, int height, int &diffValue) {

    int predT;
    int predL;
    int predTL;
    int pred;
    int left;
    int top;
    int topLeft;
    int mode;
    int actual;
    int diff;

    if (x <= 0) {
        left = 0;
        topLeft = 0;
    }
    if (y <= 0) {
        top = 0;
        topLeft = 0;
    }
    if (y > 0) {
        top = buf[x + (y - 1) * width];
    }
    if (x > 0 && y >= 0) {
        left = buf[(x - 1) + y * width];
    }
    if (x > 0 && y > 0) {
        topLeft = buf[(x - 1) + (y - 1) * width];
    }

    predT = top;
    predL = left;
    predTL = topLeft;
    actual = buf[x + y * width];

    if (predL <= predT && predL <= predTL) {
        mode = 1;
        pred = predL;
    }
    else if (predT <= predL && predT <= predTL) {
        mode = 2;
        pred = predT;
    }
    else if (predTL <= predL && predTL <= predT) {
        mode = 3;
        pred = predTL;
    }
    diff = actual - pred;
    diff = diff >= 0 ? diff : -diff;
    if (diff >= 8) {
        mode = 4;
        diffValue = actual;
    }
    else {
        diffValue = actual - pred;
    }

    return mode;
}

unsigned char predDiff(unsigned char *buf, int x, int y, int width, int height, int mode, int diffValue) {

    int predT;
    int predL;
    int predTL;
    int pred;
    int left;
    int top;
    int topLeft;

    if (x <= 0) {
        left = 0;
        topLeft = 0;
    }
    if (y <= 0) {
        top = 0;
        topLeft = 0;
    }
    if (y > 0) {
        top = buf[x + (y - 1) * width];
    }
    if (x > 0 && y >= 0) {
        left = buf[(x - 1) + y * width];
    }
    if (x > 0 && y > 0) {
        topLeft = buf[(x - 1) + (y - 1) * width];
    }
    predT = top;
    predL = left;
    predTL = topLeft;

    switch (mode) {
        case 1:
        pred = predL + diffValue;
        break;
    case 2:
        pred = predT + diffValue;
        break;
    case 3:
        pred = predTL + diffValue;
        break;
    case 4:
        pred = diffValue;
        break;
    }

    return (unsigned char)pred;
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

void CAppCompress::getFilteredImage(int *filtered_b, int *filtered_g, int *filtered_r) {
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

void CAppCompress::countDiffIntensity(int *filtered_b, int *filtered_g, int *filtered_r, int *diff_b, int *diff_g, int *diff_r) {
    int i, j;

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            int value_b = filtered_b[i + j * width];
            int value_g = filtered_g[i + j * width];
            int value_r = filtered_r[i + j * width];

            diff_b[value_b + 255] += 1;
            diff_g[value_g + 255] += 1;
            diff_r[value_r + 255] += 1;
        }
    }
}

int CAppCompress::getDiffCount(int *diff) {
    int i;
    int count = 0;

    for (i = 0; i < 511; i++) {
        if (diff[i] != 0) {
            count += 1;
        }
    }
}

void CAppCompress::splitDiffAndFreq(int *diff, int *data, int* freq) {
    int i, j;
    int idx = 0;

    for (i = 0; i < 511; i++) {
        if (diff[i] != 0) {
            data[idx] = i - 255;
            freq[idx] = diff[i];
            idx += 1;
        }
    }
}

CAppCompress::Node* CAppCompress::newNode(int data, unsigned freq) {
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

CAppCompress::MinHeap* CAppCompress::createAndBuildMinHeap(int data[], int freq[], int size) {

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

CAppCompress::Node* CAppCompress::buildHuffmanTree(int data[], int freq[], int size) {
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

void CAppCompress::traverse(CAppCompress::Node* node, string* code_dict, string code)
{
    if (node->left == NULL && node->right == NULL)
    {
        code_dict[node->diff + 255] = code;
    }
    else
    {
        traverse(node->left, code_dict, code + '0');
        traverse(node->right, code_dict, code + '1');
    }
}

// Huffman Encoding using the Huffman tree built above
void CAppCompress::HuffmanEncode() {
    int i, j;
    int dataSize = width * height;

    // different value range[-255, 255]
    int diff_b[511], diff_g[511], diff_r[511];

    // initialization array
    for (i = 0; i < 511; i++) {
        diff_b[i] = 0;
        diff_g[i] = 0;
        diff_r[i] = 0;
    }

    int *filtered_b = new int[dataSize];
    int *filtered_g = new int[dataSize];
    int *filtered_r = new int[dataSize];

    getFilteredImage(filtered_b, filtered_g, filtered_r);

    // get the different value for each channel
    countDiffIntensity(filtered_b, filtered_g, filtered_r, diff_b, diff_g, diff_r);

    // get count of each different value
    int b_count = getDiffCount(diff_b);
    int g_count = getDiffCount(diff_g);
    int r_count = getDiffCount(diff_r);

    int *data_b = new int[b_count];
    int *data_g = new int[g_count];
    int *data_r = new int[r_count];
    int *freq_b = new int[b_count];
    int *freq_g = new int[g_count];
    int *freq_r = new int[r_count];

    // split different value and frequent
    splitDiffAndFreq(diff_b, data_b, freq_b);
    splitDiffAndFreq(diff_g, data_g, freq_g);
    splitDiffAndFreq(diff_r, data_r, freq_r);

    // build Huffman tree
    Node* root_b = buildHuffmanTree(data_b, freq_b, b_count);
    Node* root_g = buildHuffmanTree(data_g, freq_g, g_count);
    Node* root_r = buildHuffmanTree(data_r, freq_r, r_count);

    // the encoding string
    encoded_sequence[0] = encoded_sequence[1] = encoded_sequence[2] = "";

    // the coding dictionary
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 511; j++)
        code_dict[i][j] = ' ';
    }

    // get code dictionary for each difference value in each channel
    traverse(root_b, code_dict[0], "");
    traverse(root_g, code_dict[1], "");
    traverse(root_r, code_dict[2], "");

    // 
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            encoded_sequence[0] += code_dict[0][filtered_b[i + j * width]];
            encoded_sequence[1] += code_dict[0][filtered_g[i + j * width]];
            encoded_sequence[2] += code_dict[0][filtered_r[i + j * width]];
        }
    }
}

// convert "101100.." to 8-bit unsigned char
void CAppCompress::convertDecodedStringToBytes(string encoded_sequence, unsigned char *encoded_data) {
    int bit_num = 0;
    unsigned char num = 7;
    unsigned char data = 0;
    int idx = 0;

    for (string::iterator it = encoded_sequence.begin(); it != encoded_sequence.end(); ++it) {
        if (*it == '1') {
            data += 1 << bit_num;
        }

        if (bit_num == 0) {
            bit_num = 7;
            encoded_data[idx] = data;
            data = 0;
            idx += 1;
        }else {
            bit_num -= 1;
        }
    }
}

void CAppCompress::HuffmanDecode(CAppCompress::Node* root, string* encoded_sequence, unsigned char** data) {
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
                }
                else (encoded_sequence[i][j] == '1') {
                 temp = temp->right;
                }
            }
        }
    }
}

void CAppCompress::DiffDecode() {

}


// This function compresses input 24-bit image (8-8-8 format, in pInput pointer).
// This function shall allocate storage space for compressedData, and return it as a pointer.
// The input reference variable cDataSize, is also serve as an output variable to indicate the size (in bytes) of the compressed data.
unsigned char *CAppCompress::Compress(int &cDataSize) {
    HuffmanEncode();

    int encoded_sequence_b_size = encoded_sequence[0].size();
    int encoded_sequence_g_size = encoded_sequence[1].size();
    int encoded_sequence_r_size = encoded_sequence[2].size();

    int encoded_data_b_count = encoded_sequence_b_size / 8;
    int encoded_data_g_count = encoded_sequence_g_size / 8;
    int encoded_data_r_count = encoded_sequence_r_size / 8;

    encoded_data_b_count = encoded_data_b_count * 8 < encoded_sequence_b_size ? encoded_data_b_count + 1 : encoded_data_b_count;
    encoded_data_g_count = encoded_data_g_count * 8 < encoded_sequence_g_size ? encoded_data_g_count + 1 : encoded_data_g_count;
    encoded_data_r_count = encoded_data_r_count * 8 < encoded_sequence_r_size ? encoded_data_r_count + 1 : encoded_data_r_count;

    unsigned char *encoded_data_b = new unsigned char[encoded_data_b_count];
    unsigned char *encoded_data_g = new unsigned char[encoded_data_g_count];
    unsigned char *encoded_data_r = new unsigned char[encoded_data_r_count];

    convertDecodedStringToBytes(encoded_sequence[0], encoded_data_b);
    convertDecodedStringToBytes(encoded_sequence[1], encoded_data_g);
    convertDecodedStringToBytes(encoded_sequence[2], encoded_data_r);



    // You can modify anything within this function, but you cannot change the function prototype.
    unsigned char *compressedData;

    cDataSize = encoded_data_b_count + encoded_data_g_count + encoded_data_r_count;    // You need to determine the size of the compressed data. 
            // Here, we simply set it to the size of the original image
    compressedData = new unsigned char[cDataSize]; // As an example, we just copy the original data as compressedData.

    memcpy(compressedData, pInput, cDataSize);

    return compressedData;  // return the compressed data
}

// This function takes in compressedData with size cDatasize, and decompresses it into 8-8-8 image.
// The decompressed image data should be stored into the uncompressedData buffer, with 8-8-8 image format
void CAppCompress::Decompress(unsigned char *compressedData, int cDataSize, unsigned char *uncompressedData) {

    // You can modify anything within this function, but you cannot change the function prototype.
    memcpy(uncompressedData, compressedData, cDataSize); // Here, we simply copy the compressedData into the output buffer.
}


void CAppCompress::Process(void) {

    // Don't change anything within this function.

    int i, cDataSize;

    unsigned char *compressedData;
    unsigned char *verifyCompressedData;

    SetTitle(pOutput, _T("Lossless Decompressed Image"));

    compressedData = Compress(cDataSize);

    verifyCompressedData = new unsigned char[cDataSize];

    memcpy(verifyCompressedData, compressedData, cDataSize);

    delete[] compressedData;

    Decompress(verifyCompressedData, cDataSize, pOutput);

    for (i = 0; i < width * height * 3; i++) {
        if (pInput[i] != pOutput[i]) {
            printf(_T("Caution: Decoded Image is not identical to the Original Image!\r\n"));
            break;
        }
    }

    printf(_T("Original Size = %d, Compressed Size = %d, Compression Ratio = %2.2f\r\n"), width * height * 3, cDataSize, (double)width * height * 3 / cDataSize);

    PutDC(pOutput);
}