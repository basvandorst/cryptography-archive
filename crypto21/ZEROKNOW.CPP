// zeroknow.cpp - written and placed in the public domain by Wei Dai

// Please do not use the Zero-Knowledge Graph Isomorphism Prover and Verifier
// for cryptographic purposes.  They are not likely to be secure and
// are here only as an illustration of how zero-knowledge proofs work.

#include "asn.h"
#include "zeroknow.h"

#include <iostream.h>

SimpleGraph::SimpleGraph(word16 size)
    : size(size), adjMatrix(new BitArray[size])
{
    for (unsigned int i=0; i<size; i++)
        adjMatrix[i].Resize(size);
}

SimpleGraph::SimpleGraph(const SimpleGraph &original)
    : size(original.size), adjMatrix(new BitArray[size])
{
    for (unsigned int i=0; i<size; i++)
        adjMatrix[i]=original.adjMatrix[i];
}

SimpleGraph::SimpleGraph(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    BitArray temp(seq);

    size = temp.Size();
    adjMatrix = new BitArray[size];
    adjMatrix[0] = temp;

    for (unsigned int i=1; i<size; i++)
        adjMatrix[i].BERDecode(seq);
}

void SimpleGraph::DEREncode(BufferedTransformation &bt)
{
    DERSequenceEncoder seq(bt);
    for (unsigned int i=0; i<size; i++)
        adjMatrix[i].DEREncode(seq);
}

void SimpleGraph::Resize(word16 newSize)
{
    if (size != newSize)
    {
        size = newSize;
        delete [] adjMatrix;
        adjMatrix = new BitArray[size];

        for (unsigned int i=0; i<size; i++)
            adjMatrix[i].Resize(size);
    }
    else
        CutAll();
}

SimpleGraph & SimpleGraph::operator=(const SimpleGraph &original)
{
    Resize(original.size);

    for (unsigned int i=0; i<size; i++)
        adjMatrix[i]=original.adjMatrix[i];

    return *this;
}

boolean SimpleGraph::operator==(const SimpleGraph &target)
{
    if (size != target.size)
        return FALSE;

    for (unsigned int i=0; i<size; i++)
        if (adjMatrix[i]!=target.adjMatrix[i])
            return FALSE;

    return TRUE;
}

SimpleGraph::~SimpleGraph()
{
    delete [] adjMatrix;
}

void SimpleGraph::Randomize(RandomNumberGenerator &rng, word16 degree)
{
    assert(degree+1 < size);

    CutAll();

    SecBlock<word16> v(size);

    for (unsigned int currentVertex=0; currentVertex+1<size; currentVertex++)
    {
        unsigned int i, d = Degree(currentVertex);
        if (d < degree)
        {
            for (i=currentVertex+1; i<size; i++)
                v[i] = i;
            Shuffle(rng, v.ptr+currentVertex+1, size-currentVertex-1);

            for (i=0; i<min(degree-d, size-currentVertex-1); i++)
                Link(currentVertex, v[currentVertex+1+i]);
        }
    }
/*
    if (size==0)
        return;

    unsigned long maxEdges = (unsigned long)size*(size-1)/2;
    assert (numberOfEdges <= maxEdges);

    if (numberOfEdges <= maxEdges/2)
    {               // start empty and form as many random edges as needed
        CutAll();
        for (unsigned long i=0; i<numberOfEdges;)
        {
            unsigned int vertex1 = rng.GetShort(0, size-1);
            unsigned int vertex2 = rng.GetShort(0, size-1);
            if ((vertex1 != vertex2) && !Adjacent(vertex1, vertex2))
            {
                Link(vertex1, vertex2);
                i++;
            }
        }
    }
    else
    {   // start with a complete graph and remove (maxEdges-numberOfEdges) random edges
        LinkAll();
        for (unsigned long i=maxEdges; i>numberOfEdges;)
        {
            unsigned int vertex1 = rng.GetShort(0, size-1);
            unsigned int vertex2 = rng.GetShort(0, size-1);
            if ((vertex1 != vertex2) && Adjacent(vertex1, vertex2))
            {
                Cut(vertex1, vertex2);
                i--;
            }
        }
    }
*/
}

void SimpleGraph::Cut(word16 vertex)
{
    for (unsigned int i=0; i<size; i++)
        Cut(i, vertex);
}

void SimpleGraph::CutAll()
{
    for (unsigned int i=0; i<size; i++)
        adjMatrix[i].ClearAll();
}

void SimpleGraph::LinkAll()
{
    for (unsigned int i=0; i<size; i++)
    {
        adjMatrix[i].SetAll();
        adjMatrix[i][i] = 0;    // remove loop
    }
}

void SimpleGraph::Permutate(const word16 *isomorphism)
{
#ifndef NDEBUG
    {   // check to make sure isomorphism really is a permutation
        unsigned long sumCheck=0;
        unsigned int xorCheck=0;
		unsigned int i;
        for (i=0; i<size; i++)
        {
            assert (isomorphism[i] < size);
            xorCheck ^= isomorphism[i];
            sumCheck += isomorphism[i];
        }
        assert (sumCheck==(unsigned long)size*(size-1)/2);
        unsigned long nextPowerOf2 = 1L<<BitPrecision(size);
        for (; i<nextPowerOf2; i++)
            xorCheck ^= i;
        assert (xorCheck == 0);
    }
#endif

    unsigned int i;
    SecBlock<word16> perm(size);

    // swap rows according to isomorphism
    memcpy(perm, isomorphism, 2*size);
    for (i=0; i<size; i++)
        while (perm[i] != i)
        {
            swapBitArray(adjMatrix[i], adjMatrix[perm[i]]);
            swap(perm[i], perm[perm[i]]);
        }

    // swap columns
    memcpy(perm, isomorphism, 2*size);
    for (i=0; i<size; i++)
        while (perm[i] != i)
        {
            for (unsigned int j=0; j<size; j++)
                adjMatrix[j].swapElement(i, perm[i]);
            swap(perm[i], perm[perm[i]]);
        }
}

ostream& operator<<(ostream& out, const SimpleGraph &a)
{
/*
    if (a.size>=10)
    {
        out << "   ";
        for (word16 i=0; i<a.size; i++)
            out << (i/10 ? i/10 : ' ');
        out << endl;
    }

    out << "   ";
    for (word16 i=0; i<a.size; i++)
        out << i%10;
    out << endl << endl;

    for (i=0; i<a.size; i++)
        out << setw(2) << i << ' ' << a.adjMatrix[i] << endl;
*/
    for (unsigned int i=0; i<a.size; i++)
        out << a.adjMatrix[i] << endl;

    return out;
}

ZK_IsomorphismProver::ZK_IsomorphismProver(const SimpleGraph &g1,
        const word16 *g1_to_g2_isomorphism, RandomNumberGenerator &rng)
    : g1(g1), g1_to_g2(g1.Size()), g1_to_h(g1.Size()), g2_to_h(g1.Size()), rng(rng)
{
    memcpy(g1_to_g2, g1_to_g2_isomorphism, 2*g1.Size());
}

void ZK_IsomorphismProver::Setup(SimpleGraph &h)
{
    // create a random isomorphism from g2 to h
	unsigned int i;
    for (i=0; i<g1.Size(); i++)
        g2_to_h[i]=i;
    Shuffle(rng, (word16 *)g2_to_h, (unsigned int)g1.Size());

    // calculate g1_to_h based on g1_to_g2 and g2_to_h
    for (i=0; i<g1.Size(); i++)
        g1_to_h[i] = g2_to_h[g1_to_g2[i]];

    h = g1;
    h.Permutate(g1_to_h);
}

void ZK_IsomorphismProver::Prove(int i, word16 *gi_to_h_isomorphism)
{
    assert(i==1 || i==2);
    if (i==1)
    {
        memcpy(gi_to_h_isomorphism, g1_to_h, 2*g1.Size());
        memset(g2_to_h, 0, 2*g1.Size());
    }
    else
    {
        memcpy(gi_to_h_isomorphism, g2_to_h, 2*g1.Size());
        memset(g1_to_h, 0, 2*g1.Size());
    }
}

ZK_IsomorphismVerifier::ZK_IsomorphismVerifier(const SimpleGraph &g1, const SimpleGraph &g2, RandomNumberGenerator &rng)
    : g1(g1), g2(g2), h(g1.Size()), rng(rng)
{
    assert(g1.Size() == g2.Size());
}

int ZK_IsomorphismVerifier::Setup(const SimpleGraph &hIn)
{
    h = hIn;
    chosenGraph = rng.GetBit() + 1;
    return chosenGraph;
}

boolean ZK_IsomorphismVerifier::Verify(const word16 *gi_to_h)
{
    assert(chosenGraph == 1 || chosenGraph == 2);

    if (chosenGraph==1)
        return VerifyIsomorphism(g1, h, gi_to_h);
    else
        return VerifyIsomorphism(g2, h, gi_to_h);
}

boolean ZK_IsomorphismVerifier::VerifyIsomorphism(SimpleGraph g1, const SimpleGraph &g2, const word16 *g1_to_g2)
{
    g1.Permutate(g1_to_g2);
    return g1==g2;
}

