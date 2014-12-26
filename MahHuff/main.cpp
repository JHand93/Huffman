/*
 * Author: John Handley
 * Assignment Title: rehuff
 * Assignment Description: This program uses huffman encoding to encode and
 *                         decode a file so that it may be compressed and/or
 *                         decompressed for the user using a Tree and a priority
 *                         queue to create the huffman tree for encoding and
 *                         decoding purposes.
 * Due Date: 4/13/14
 * Date Created: 4/8/14
 * Date Last Modified: 4/13/14
 */
#include<iostream>
#include<fstream>
#include<string>
#include<iomanip> //for width()
#include<stdlib.h>
#include<string.h>
//#include<queue> // doesnt work on tree

using namespace std;

const unsigned short MAGIC_NUMBER = 6706;

//priority queue
template<class T>
class Queue{
    private:
        T* *arr;
        int back; //the last element in the queue
        int size; //current size of the queue array
        static const int SIZE = 10; //size increment step size
        int maxChild; //max number of children for a parent>=2
    
        Queue(const Queue &);
        const Queue & operator=(const Queue &);
    
        void heapup(int, int);
        void heapdown(int, int);
        void swap(T* &, T* &);
    
    public:
        Queue(int d = 2);
        ~Queue(void);
        void enq(T*);
        T* deq(void);
        T* front(void);
        bool empty(void) const;
        bool full(void) const;
};

class Node{
    public:
        unsigned int freq;
        unsigned char ch;
        Node *left, *right;
        //constructor
        Node(void){
            freq = 0;
            ch = '\0';
            left = NULL;
            right = NULL;
        }
};

// Huffman Tree
class Tree{
private:
    Node *root;
    
    Tree(const Tree &);
    const Tree & operator=(const Tree &);
    void clear(Node * N);
    
public:
    Tree();
    ~Tree();
    
    unsigned int getFreq(void) const;
    unsigned char getChar(void) const;
    void setFreq(unsigned int);
    void setChar(unsigned char);
    Node* getLeft(void) const;
    Node* getRight(void) const;
    void setLeft(Node *);
    void setRight(Node *);
    Node* getRoot(void) const;

    bool operator==(const Tree &) const;
    bool operator!=(const Tree &) const;
    bool operator<(const Tree &) const;
    bool operator>(const Tree &) const;
    bool operator<=(const Tree &) const;
    bool operator>=(const Tree &) const;
    
    //to get Huffman string of a given character
    void huf(Node *, unsigned char, string, string &) const;
    //outputs the Huffman char-string pairs list
    void huf_list(Node *, string) const;
    //to get char equivalent of a Huffman string(if present)
    bool findChar(string, unsigned char &) const;
    string printChar(Node *) const; //prints chars to file
};

//read a bit from the stream(file)
unsigned char readHuff(ifstream & infile);
//write a passed bit to the stream(file)
void writeHuff(unsigned char i, ofstream & outfile);

int main(int argc, const char * argv[])
{
    if(argc != 4){
        cerr << "improper number of arguments" << endl;
        cerr << argv[0] << " proper usage: -[un]huff <source> <destination>" << endl;
        exit(1);
    }
    
    if(strcmp(argv[1], "-huff") == 0){
        ifstream inFile(argv[2], ios::in|ios::binary);
        //open source
        if(!inFile){
            cerr << argv[2] << " could not be opened" << endl;
            exit(1);
        }
        
        //open destination
        ofstream outFile(argv[3], ios::out|ios::binary);
        if(!outFile){
            cerr << argv[3] << " could not be opened" << endl;
            exit(1);
        }
        
        streampos beg, end;
        beg = inFile.tellg();
        inFile.seekg (0, ios::end);
        end = inFile.tellg();
        inFile.seekg(0, ios::beg);
        
        unsigned int fileSize = (unsigned int)(end-beg);
        
        //hold all readable values from file
        unsigned int f[256];
        for(int i = 0; i < 256; i++){
            f[i] = 0;
        }
        
        //get frequency of all symbols
        char c;
        unsigned char ch;
        while(inFile.get(c)){
            ch = c;
            f[ch]++;
        }
        
        inFile.clear(); //clear EOF flag
        inFile.seekg(0); //back to beginning
        
        Queue<Tree> q(3);
        Tree* tree;
        
        //Put the magic number in the file
        outFile.write(reinterpret_cast<const char*>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));
        
        for(int i = 0; i < 256; i++){
            //output char freq table to the output file
            //divide 32 bit unsigned int values into 4 bytes
            outFile.put(static_cast<unsigned char>(f[i]>>24));
            outFile.put(static_cast<unsigned char>((f[i]>>16)%256));
            outFile.put(static_cast<unsigned char>((f[i]>>8)%256));
            outFile.put(static_cast<unsigned char>(f[i]%256));
            
            if(f[i] > 0){
                //send freq-char pairs to the priority heap as Huffman trees
                tree=new Tree;
                (*tree).setFreq(f[i]);
                (*tree).setChar(static_cast<unsigned char>(i));
                q.enq(tree);
            }
        }
        
        //construct the main Huffman Tree
        Tree* tree2;
        Tree* tree3;
        
        do{
            tree = q.deq();
            if(!q.empty()){
                tree2=q.deq();
                tree3=new Tree;
                (*tree3).setFreq((*tree).getFreq()+(*tree2).getFreq());
                (*tree3).setLeft((*tree).getRoot());
                (*tree3).setRight((*tree2).getRoot());
                q.enq(tree3);
            }
        }
        while(!q.empty()); //until sub-trees combined
        
        //find Huffman strings of all chars in the Huffman tree and put into a string table
        string table[256];
        unsigned char uchar;
        for(unsigned short uShort = 0; uShort < 256; uShort++){
            table[uShort] = "";
            uchar = static_cast<unsigned char>(uShort);
            (*tree).huf((*tree).getRoot(), uchar, "", table[uShort]);
        }
        
        //output the huffman code into the file
        unsigned char ch2;
        while(inFile.get(c)){
            ch = c;
            
            //send the Huffman string to output file bit by bit
            for(unsigned int i = 0; i < table[ch].size(); i++){
                if(table[ch].at(i)=='0'){
                    ch2=0;
                }
                if(table[ch].at(i)=='1'){
                    ch2=1;
                }
                writeHuff(ch2, outFile);
            }
        }
        ch2 = 2; //EOF KEY
        writeHuff(ch2, outFile);
        
        outFile.seekp(0, ios::beg);
        beg = outFile.tellp();
        outFile.seekp(0, ios::end);
        end = outFile.tellp();
        
        if(fileSize <= (end-beg)){
            cerr << "File will not compress" << endl;
        }
        
        inFile.close();
        outFile.close();
    }
    else if(strcmp(argv[1], "-unhuff") == 0){
        ifstream inFile(argv[2], ios::in|ios::binary);
        if(!inFile){
            cerr << argv[2] << " could not be opened" << endl;
            exit(1);
        }
        
        //open the output file
        ofstream outFile(argv[3], ios::out|ios::binary);
        if(!outFile){
            cerr << argv[3] << " could not be opened" << endl;
            exit(1);
        }

        
        // Confirm magic number
        unsigned short magic_number = 0;
        inFile.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
        
        if(magic_number != MAGIC_NUMBER){
            cerr << "Input file was not Huffman Endoded." << endl;
            exit(1);
        }
        
        //read frequency table from the input file
        unsigned int f[256];
        char c;
        unsigned char ch;
        unsigned int j=1;
        for(int i = 0 ;i < 256; i++){
            //read 4 bytes and combine them into one 32 bit unsigned int value
            f[i] = 0;
            for(int k = 3; k >= 0; k--){
                inFile.get(c);
                ch = c;
                f[i] += ch*(j<<(8*k));
            }
        }

        //re-construct the Huffman tree
        Queue<Tree> q(3);
        Tree* tree;
        
        for(int i = 0; i < 256; i++){
            if(f[i] > 0){
                tree = new Tree;
                (*tree).setFreq(f[i]);
                (*tree).setChar(static_cast<unsigned char>(i));
                q.enq(tree);
            }
        }
        Tree* tree2;
        Tree* tree3;
        
        do{
            tree = q.deq();
            if(!q.empty()){
                tree2=q.deq();
                tree3=new Tree;
                (*tree3).setFreq((*tree).getFreq()+(*tree2).getFreq());
                (*tree3).setLeft((*tree).getRoot());
                (*tree3).setRight((*tree2).getRoot());
                q.enq(tree3);
            }
        }
        while(!q.empty()); //until huff is done
        
        string huffString;
        unsigned char ch2;
        unsigned int total = (*tree).getFreq();
        
        //continue until no char left to unhuff
        while(total > 0){
            huffString = ""; //current Huffman string
            do{
                //read Huffman strings bit by bit
                ch = readHuff(inFile);
                if(ch == 0){
                    huffString = huffString + '0';
                }
                if(ch == 1){
                    huffString = huffString + '1';
                }
            }
            while(!(*tree).findChar(huffString, ch2));
            
            outFile.put(static_cast<char>(ch2));
            total--;
        }
        
        inFile.close();
        outFile.close();
    }
    else{
        cerr << argv[0] << " requires -[un]huff as a command" << endl;
        exit(1);
    }

    return 0;
}

// constructor (creates a new queue)
template<class T>
Queue<T>::Queue(int d){
    if(d < 2){
       d = 2; //min a 2-heap is supported
    }
    maxChild = d;
    back = 0;
    size = SIZE;
    arr = new T*[size];
}

//empty?
template<class T>
bool Queue<T>::empty(void) const{
    return (back <= 0);
}

//full?
template<class T>
bool Queue<T>::full(void) const{
    return (back >= size);
}

//the front element of the queue(removed from queue)
template<class T>
T* Queue<T>::deq(void){
    if(empty()){
        cerr << "File will not compress" << endl;
        exit(1);
    }
    
    T* val = arr[0];
    arr[0]=arr[back-1];
    back--;
    heapdown(0, back-1);
    
    return val;
}

//a copy of the front element is returned but the queue is unchanged
template<class T>
T* Queue<T>::front(void){
    if(empty()){
        cerr << "File will not compress" << endl;
        exit(1);
    }
    return arr[0];
}

// add to queue
template<class T>
void Queue<T>::enq(T* element){
    //if the array is full then make it larger
    if(full()){ 
        int size2 = size + SIZE; //the size of the new array
        T* *temp = new T*[size2]; //new array
        
        //copy old array to the new one
        for(int i = 0; i < size; i++){
            temp[i] = arr[i];
        }
        delete[] arr;
        arr = temp;
        size = size2;
    }
    
    arr[back] = element;
    back++;
    
    heapup(0, back-1);
}

//upwardly fixes queue after a new object is added
template<class T>
void Queue<T>::heapup(int root, int bottom){
    int parent; //parent node
    
    if(bottom > root){
        parent = (bottom-1)/maxChild;
        
        if(*arr[parent] > *arr[bottom]){
            swap(arr[parent], arr[bottom]);
            heapup(root, parent);
        }
    }
}

//downwardly fixes queue after a new object is added
template<class T>
void Queue<T>::heapdown(int root, int bottom){
    int minc, firstc, c;
    
    firstc = root*maxChild + 1; // first child
    
    if(firstc <= bottom){
        minc = firstc;
        
        for(int i = 2; i <= maxChild; i++){
            c = root*maxChild + i; //pos of next c
            if(c <= bottom){
                if(*arr[c] < *arr[minc]){
                    minc = c;
                }
            }
        }
        
        if(*arr[root] > *arr[minc]){
            swap(arr[root], arr[minc]);
            heapdown(minc, bottom);
        }
    }
}

// swaps to variables
template<class T>
void Queue<T>::swap(T* &obj1, T* &obj2){
    T* temp;
    temp = obj1;
    obj1 = obj2;
    obj2 = temp;
}

// destructor(also deallocates)
template<class T>
Queue<T>::~Queue(void){
    delete[] arr;
}

//constructor for tree
Tree::Tree(void){
    Node* theNode = new Node;
    root = theNode;
}

//recursive func to delete the tree from memory
void Tree::clear(Node *theNode){
    if(theNode){
        clear(theNode->left);
        clear(theNode->right);
        delete theNode;
    }
}

//destructor for tree
Tree::~Tree(void){
    clear(root);
}


//Accessor Functions
unsigned int Tree::getFreq(void) const{
    return root->freq;
}
unsigned char Tree::getChar(void) const{
    return root->ch;
}
void Tree::setFreq(unsigned int f){
    root->freq=f;
}
void Tree::setChar(unsigned char c){
    root->ch=c;
}
void Tree::setLeft(Node* N){
    root->left=N;
}
void Tree::setRight(Node* N){
    root->right=N;
}
Node* Tree::getRoot(void) const{
    return root;
}
Node* Tree::getLeft(void) const{
    return root->left;
}
Node* Tree::getRight(void) const{
    return root->right;
}

//Operator Functions
bool Tree::operator<(const Tree & T) const{
    return (root->freq < T.root->freq);
}
bool Tree::operator>(const Tree & T) const{
    return (root->freq > T.root->freq);
}
bool Tree::operator<=(const Tree & T) const{
    return (root->freq <= T.root->freq);
}
bool Tree::operator>=(const Tree & T) const{
    return (root->freq >= T.root->freq);
}
bool Tree::operator==(const Tree & T) const{
    return (root->freq == T.root->freq);
}
bool Tree::operator!=(const Tree & T) const{
    return (root->freq != T.root->freq);
}

void Tree::huf(Node* N, unsigned char c, string str, string & theString) const{
    if(N){ //if node isnt null
        //compare char of the leaf node to given char
        if(!N->left && !N->right && N->ch == c){
            theString = str; //if the character exists then copy the string
        }
        else{
            huf(N->left, c, str + "0", theString);
            huf(N->right, c, str + "1", theString);
        }
    }
}
bool Tree::findChar(string theString, unsigned char & c) const{
    Node * curr = root;
    
    for(unsigned int i = 0; i < theString.size(); i++){
        if(theString[i] == '0') //go left
            curr = curr->left;
        if(theString[i] == '1') //go right
            curr = curr->right;
    }
    
    bool found = false;
    
    if(!curr->left && !curr->right){ //if leaf
        found = true;
        c = curr->ch;
    }
    
    return found;
}
string Tree::printChar(Node * N) const{
    string s = "";
    
    if(!N->left && !N->right){ //if its a leaf
        unsigned char c = N->ch;
        
        //if char cant be printed then output its octal ASCII code
        if(iscntrl(c) || c==32){ //32 == blank chararacter
            //find octal code of the character (3 digits)
            char* cp = new char;
            for(int i = 0; i < 3; i++){
                sprintf(cp, "%i", c % 8);
                c -= c % 8;
                c /= 8;
                s = (*cp) + s;
            }
            s = '/' + s; // add \ before the octal code
        }
        else{
            s = c;
        }
    }
    return s;
}


void writeHuff(unsigned char i, ofstream & outFile){
    static int bitPos = 0;
    static unsigned char c = '\0';
    
    if(i < 2){
        if(i == 1){
            c = c | (i << (7-bitPos));
        }
        else{
            c = c & static_cast<unsigned char> (255-(1 << (7-bitPos)));
        }
        
        bitPos++;
        bitPos %= 8;
        
        if(bitPos == 0){
            outFile.put(c);
            c = '\0';
        }
    }
    else{
        outFile.put(c);
    }
}
unsigned char readHuff(ifstream & inFile){
    static int bitPos = 0;
    static unsigned char c = inFile.get();
    unsigned char i;
    
    i = (c >> (7-bitPos)) % 2;
    bitPos++;
    bitPos %= 8;
    
    if(bitPos == 0){
        if(!inFile.eof()){
            c = inFile.get();
        }
        else{
            i = 2;
        }
    }
    return i;
}