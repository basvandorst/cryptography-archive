#ifndef ZEROKNOW_H
#define ZEROKNOW_H

#include "cryptlib.h"
#include "misc.h"
#include "bitarray.h"
#include <memory.h>
#include <assert.h>

class ostream;

class SimpleGraph
{
public:
    SimpleGraph(word16 numberOfVertices=0);   // starts out with no edges
    SimpleGraph(const SimpleGraph &);
    ~SimpleGraph();

    SimpleGraph(BufferedTransformation &in);
    void DEREncode(BufferedTransformation &out);

    void Resize(word16 newSize);

    SimpleGraph & operator=(const SimpleGraph &);
    boolean operator==(const SimpleGraph &);

    // randomize the graph, will try to let most vertices have the same
    // desired degree (to make finding isomorphisms harder)
    void Randomize(RandomNumberGenerator &rng, word16 degree);

    word16 Size() const
    {
        return size;
    }

    word16 Degree(word16 vertex)
    {
        return adjMatrix[vertex].Count();
    }

    boolean Adjacent(word16 vertex1, word16 vertex2)
    {
        assert(vertex1 < size && vertex2 < size);
        return adjMatrix[vertex1][vertex2];
    }

    void Link(word16 vertex1, word16 vertex2)
    {
        assert(vertex1 < size && vertex2 < size);
        assert(vertex1 != vertex2); // loops not allowed
        adjMatrix[vertex1][vertex2] = 1;
        adjMatrix[vertex2][vertex1] = 1;
    }

    void Cut(word16 vertex1, word16 vertex2)
    {
        assert(vertex1 < size && vertex2 < size);
        adjMatrix[vertex1][vertex2] = 0;
        adjMatrix[vertex2][vertex1] = 0;
    }

    void Cut(word16 vertex);

    void LinkAll(); // form a complete graph
    void CutAll();  // remove all edges

    void Permutate(const word16 *isomorphism);

    friend ostream& operator<<(ostream& out, const SimpleGraph &a);

private:
    word16 size;
    BitArray *adjMatrix;
};

/*
    This is what one round of a zero knowledge proof for graph
    isomorphis should look like, assuming prover and verifier
    are running on different machines.  Comments indicate what still
    needs to be implemented.

    prover.Setup(h);
    // send graph h to verifier

                                i = verifier.Setup(h);
                                // send i (number of the graph chosen) to prover

    prover.Prove(i, gi_to_h);
    // send isomorphism gi_to_h to verifier

                                verified = verifier.Verify(gi_to_h);
*/

class ZK_IsomorphismProver
{
public:
    // g1_to_g2 should be a mapping from g1's vertices to g2's vertices
    // each element of the array j=g1_to_g2[i] should specify an assignment
    // from g1's i-th vertex to g2's j-th vertex
    ZK_IsomorphismProver(const SimpleGraph &g1, const word16 *g1_to_g2,
                         RandomNumberGenerator &rng);

    void Setup(SimpleGraph &h);
    void Prove(int i, word16 *gi_to_h);

private:
    SimpleGraph g1;
    SecBlock<word16> g1_to_g2, g1_to_h, g2_to_h;
    RandomNumberGenerator &rng;
};

class ZK_IsomorphismVerifier
{
public:
    ZK_IsomorphismVerifier(const SimpleGraph &g1, const SimpleGraph &g2,
                           RandomNumberGenerator &rng);

    int Setup(const SimpleGraph &h);
    boolean Verify(const word16 *gi_to_h);

    static boolean VerifyIsomorphism(SimpleGraph g1, const SimpleGraph &g2,
                                     const word16 *g1_to_g2);

private:
    const SimpleGraph g1;
    const SimpleGraph g2;
    SimpleGraph h;
    int chosenGraph;
    RandomNumberGenerator &rng;
};

#endif

