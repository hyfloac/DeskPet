#pragma once

#include "Objects.hpp"
#include <cstdint>
#include <SysLib.h>
#include <type_traits>
#include <utility>

template<typename T>
T maxT(const T a, const T b) noexcept
{
    return a > b ? a : b;
}

enum class InsertMethod
{
    Ignore = 0,
    Replace,
    Greater,
    Lesser
};

enum class IteratorMethod
{
    TopDownLR = 0,
    TopDownRL,
    HighestToLowest,
    LowestToHighest
};

template<typename T, typename HeightT>
struct AVLNode final
{
    DEFAULT_DESTRUCT(AVLNode);
    DEFAULT_CM_PU(AVLNode);
public:
    AVLNode* Left;
    AVLNode* Right;
    HeightT Height;
    T Value;
public:
    AVLNode(AVLNode* const left, AVLNode* const right, const HeightT height, const T& value) noexcept
        : Left(left)
        , Right(right)
        , Height(height)
        , Value(value)
    { }

    AVLNode(AVLNode* const left, AVLNode* const right, const HeightT height, T&& value) noexcept
        : Left(left)
        , Right(right)
        , Height(height)
        , Value(::std::move(value))
    { }

    template<typename... Args>
    AVLNode(AVLNode* const left, AVLNode* const right, const HeightT height, Args&&... args) noexcept
        : Left(left)
        , Right(right)
        , Height(height)
        , Value(::std::forward<Args>(args)...)
    { }
};

template<typename T, typename HeightT, InsertMethod InsertMethod = InsertMethod::Ignore>
class AVLTree final
{
    DELETE_CM(AVLTree);
public:
    using Node = AVLNode<T, HeightT>;
private:
    Node* m_Root;
public:
    AVLTree() noexcept
        : m_Root(nullptr)
    { }

    ~AVLTree() noexcept
    {
        DisposeTree();
    }

    [[nodiscard]] Node* Root()       noexcept { return m_Root; }
    [[nodiscard]] const Node* Root() const noexcept { return m_Root; }

    template<typename SearchT>
    [[nodiscard]] T* Find(const SearchT& search) noexcept
    {
        Node* const node = Find(m_Root, search);
        if(!node)
        {
            return nullptr;
        }

        return &node->Value;
    }

    template<typename SearchT>
    [[nodiscard]] const T* Find(const SearchT& search) const noexcept
    {
        const Node* const node = Find(m_Root, search);
        if(!node)
        {
            return nullptr;
        }

        return &node->Value;
    }

    template<typename SearchT>
    [[nodiscard]] T* FindClosestMatchAbove(const SearchT& search) noexcept
    {
        Node* const node = FindClosestMatchAbove(m_Root, search);
        if(!node)
        {
            return nullptr;
        }

        return &node->Value;
    }

    template<typename SearchT>
    [[nodiscard]] const T* FindClosestMatchAbove(const SearchT& search) const noexcept
    {
        const Node* const node = FindClosestMatchAbove(m_Root, search);
        if(!node)
        {
            return nullptr;
        }

        return &node->Value;
    }

    template<typename SearchT>
    [[nodiscard]] T* FindClosestMatchBelow(const SearchT& search) noexcept
    {
        Node* const node = FindClosestMatchBelow(m_Root, search);
        if(!node)
        {
            return nullptr;
        }

        return &node->Value;
    }

    template<typename SearchT>
    [[nodiscard]] const T* FindClosestMatchBelow(const SearchT& search) const noexcept
    {
        const Node* const node = FindClosestMatchBelow(m_Root, search);
        if(!node)
        {
            return nullptr;
        }

        return &node->Value;
    }

    template<typename SearchT>
    [[nodiscard]] Node* Get(const SearchT& search) noexcept
    {
        return Find(m_Root, search);
    }

    template<typename SearchT>
    [[nodiscard]] const Node* Get(const SearchT& search) const noexcept
    {
        return Find(m_Root, search);
    }

    template<typename SearchT>
    [[nodiscard]] Node* GetClosestMatchAbove(const SearchT& search) noexcept
    {
        return FindClosestMatchAbove(m_Root, search);
    }

    template<typename SearchT>
    [[nodiscard]] const Node* GetClosestMatchAbove(const SearchT& search) const noexcept
    {
        return FindClosestMatchAbove(m_Root, search);
    }

    template<typename SearchT>
    [[nodiscard]] Node* GetClosestMatchBelow(const SearchT& search) noexcept
    {
        return FindClosestMatchBelow(m_Root, search);
    }

    template<typename SearchT>
    [[nodiscard]] const Node* GetClosestMatchBelow(const SearchT& search) const noexcept
    {
        return FindClosestMatchBelow(m_Root, search);
    }

    Node* Insert(const T& value) noexcept
    {
        if(!m_Root)
        {
            m_Root = new Node(nullptr, nullptr, 0, value);
            return m_Root;
        }
        else
        {
            Node* newNode = new Node(nullptr, nullptr, 0, value);
            m_Root = Insert(m_Root, newNode);
            return newNode;
        }
    }

    Node* Insert(T&& value) noexcept
    {
        if(!m_Root)
        {
            m_Root = new Node(nullptr, nullptr, 0, ::std::move(value));
            return m_Root;
        }
        else
        {
            Node* newNode = new Node(nullptr, nullptr, 0, ::std::move(value));
            m_Root = Insert(m_Root, newNode);
            return newNode;
        }
    }

    template<typename... Args>
    Node* Emplace(Args&&... args) noexcept
    {
        if(!m_Root)
        {
            m_Root = new Node(nullptr, nullptr, 0, ::std::forward<Args>(args)...);
            return m_Root;
        }
        else
        {
            Node* newNode = new Node(nullptr, nullptr, 0, ::std::forward<Args>(args)...);
            m_Root = Insert(m_Root, newNode);
            return newNode;
        }
    }

    template<typename SearchT>
    void Remove(const SearchT& search) noexcept
    {
        m_Root = Remove(&m_Root, search);
    }

    void Remove(const Node* search) noexcept
    {
        m_Root = Remove(&m_Root, search);
    }

    void DisposeTree() noexcept
    {
        DisposeTree(m_Root);
    }

    template<typename FuncT, IteratorMethod IteratorMethod = IteratorMethod::TopDownLR>
    void Iterate(const FuncT& func) noexcept
    {
        IterateStatic<Node, FuncT, IteratorMethod>(m_Root, func);
    }

    template<typename FuncT, IteratorMethod IteratorMethod = IteratorMethod::TopDownLR>
    void Iterate(const FuncT& func) const noexcept
    {
        IterateStatic<Node, FuncT, IteratorMethod>(m_Root, func);
    }

    template<typename ClassC, typename FuncT, IteratorMethod IteratorMethod = IteratorMethod::TopDownLR>
    void Iterate(ClassC* instance, const FuncT& func) noexcept
    {
        IterateStatic<Node, ClassC, FuncT, IteratorMethod>(m_Root, instance, func);
    }

    template<typename ClassC, typename FuncT, IteratorMethod IteratorMethod = IteratorMethod::TopDownLR>
    void Iterate(ClassC* instance, const FuncT& func) const noexcept
    {
        IterateStatic<Node, ClassC, FuncT, IteratorMethod>(m_Root, instance, func);
    }
private:
    template<typename NodeT, typename SearchT>
    [[nodiscard]] static NodeT* Find(NodeT* const tree, const SearchT& search) noexcept
    {
        NodeT* node = tree;

        while(node)
        {
            if(search == node->Value)
            {
                return node;
            }

            if(search > node->Value)
            {
                node = node->Right;
            }
            else
            {
                node = node->Left;
            }
        }

        return nullptr;
    }

    template<typename NodeT, typename SearchT>
    [[nodiscard]] static NodeT* FindClosestMatchAbove(NodeT* const tree, const SearchT& search) noexcept
    {
        NodeT* contender = tree;
        NodeT* node = tree;

        while(node)
        {
            if(search == node->Value)
            {
                return node;
            }

            if(search > node->Value)
            {
                node = node->Right;
            }
            else
            {
                if(contender->Value > node->Value)
                {
                    contender = node;
                }

                node = node->Left;
            }
        }

        return contender;
    }

    template<typename NodeT, typename SearchT>
    [[nodiscard]] static NodeT* FindClosestMatchBelow(NodeT* const tree, const SearchT& search) noexcept
    {
        NodeT* contender = tree;
        NodeT* node = tree;

        while(node)
        {
            if(search == node->Value)
            {
                return node;
            }

            if(search > node->Value)
            {
                if(contender->Value < node->Value)
                {
                    contender = node;
                }

                node = node->Right;
            }
            else
            {
                node = node->Left;
            }
        }

        return contender;
    }

    [[nodiscard]] static Node* RotateRight(Node* const pivot) noexcept
    {
        Node* newRoot = pivot->Left;
        Node* transferNode = newRoot->Right;

        newRoot->Right = pivot;
        pivot->Left = transferNode;

        pivot->Height = maxT(Height(pivot->Left), Height(pivot->Right)) + 1;
        newRoot->Height = maxT(Height(newRoot->Left), Height(newRoot->Right)) + 1;

        return newRoot;
    }

    [[nodiscard]] static Node* RotateLeft(Node* const pivot) noexcept
    {
        Node* newRoot = pivot->Right;
        Node* transferNode = newRoot->Left;

        newRoot->Left = pivot;
        pivot->Right = transferNode;

        pivot->Height = maxT(Height(pivot->Left), Height(pivot->Right)) + 1;
        newRoot->Height = maxT(Height(newRoot->Left), Height(newRoot->Right)) + 1;

        return newRoot;
    }

    [[nodiscard]] static HeightT Height(const Node* const tree) noexcept
    {
        if(!tree)
        {
            return 0;
        }

        return tree->Height;
    }

    [[nodiscard]] static int ComputeBalance(const Node* const tree) noexcept
    {
        if(!tree)
        {
            return 0;
        }

        return static_cast<int>(Height(tree->Left) - Height(tree->Right));
    }

    [[nodiscard]] static Node* Insert(Node* const tree, Node* const newNode) noexcept
    {
        if(!tree)
        {
            return newNode;
        }

        if(newNode->Value < tree->Value)
        {
            tree->Left = Insert(tree->Left, newNode);
        }
        else if(newNode->Value > tree->Value)
        {
            tree->Right = Insert(tree->Right, newNode);
        }
        else
        {
            if constexpr(InsertMethod == InsertMethod::Ignore)
            {
                delete newNode;
                return tree;
            }
            else if constexpr(InsertMethod == InsertMethod::Replace)
            {
                newNode->Left = tree->Left;
                newNode->Right = tree->Right;
                newNode->Height = tree->Height;
                delete tree;
                return newNode;
            }
            else if constexpr(InsertMethod == InsertMethod::Greater)
            {
                tree->Right = Insert(tree->Right, newNode);
            }
            else if constexpr(InsertMethod == InsertMethod::Lesser)
            {
                tree->Left = Insert(tree->Left, newNode);
            }
            else
            {
                return tree;
            }
        }

        tree->Height = maxT(Height(tree->Left), Height(tree->Right)) + 1;

        const HeightT balance = ComputeBalance(tree);

        // Left Left
        if(balance > 1 && newNode->Value < tree->Left->Value)
        {
            return RotateRight(tree);
        }

        // Right Right
        if(balance < -1 && newNode->Value > tree->Right->Value)
        {
            return RotateLeft(tree);
        }

        // Left Right
        if(balance > 1 && newNode->Value > tree->Left->Value)
        {
            tree->Left = RotateLeft(tree->Left);
            return RotateRight(tree);
        }

        // Right Left
        if(balance < -1 && newNode->Value < tree->Right->Value)
        {
            tree->Right = RotateRight(tree->Right);
            return RotateLeft(tree);
        }

        return tree;
    }

    [[nodiscard]] static Node** MinValueNode(Node** const tree) noexcept
    {
        Node* parent = nullptr;
        Node* current = *tree;
        while(current->Left)
        {
            parent = current;
            current = current->Left;
        }

        if(!parent)
        {
            return tree;
        }

        return &parent->Left;
    }

    template<typename SearchT>
    [[nodiscard]] static Node* Remove(Node** const rootHolder, const SearchT& search) noexcept
    {
        if(!rootHolder || !(*rootHolder))
        {
            return nullptr;
        }

        Node* root = *rootHolder;

        if(search < root->Value)
        {
            root->Left = Remove(&root->Left, search);
        }
        else if(search > root->Value)
        {
            root->Right = Remove(&root->Right, search);
        }
        else
        {
            if(!root->Left || !root->Right)
            {
                Node* tmp = root->Left ? root->Left : root->Right;
                if(!tmp)
                { *rootHolder = nullptr; }
                else
                { *rootHolder = tmp; }
                delete root;
                root = *rootHolder;
            }
            else
            {
                Node** tmp = MinValueNode(&root->Right);

                *rootHolder = *tmp;                 // Replace root
                *tmp = (*tmp)->Right;               // Store tmp's right branch in tmp's parent left branch
                (*rootHolder)->Left = root->Left;   // Set tmp's left branch to the old root's left branch
                (*rootHolder)->Right = root->Right; // Set tmp's right branch to the old root's right branch
                delete root;                        // Destroy root 
                root = *rootHolder;                 // Update the actual root variable
            }
        }

        if(!root)
        {
            return nullptr;
        }

        root->Height = maxT(Height(root->Left), Height(root->Right)) + 1;

        const HeightT balance = ComputeBalance(root);

        // Left Left
        if(balance > 1 && ComputeBalance(root->Left) >= 0)
        {
            return RotateRight(root);
        }

        // Left Right
        if(balance > 1 && ComputeBalance(root->Left) < 0)
        {
            root->Left = RotateLeft(root->Left);
            return RotateRight(root);
        }

        // Right Right
        if(balance < -1 && ComputeBalance(root->Right) <= 0)
        {
            return RotateLeft(root);
        }

        // Right Left
        if(balance < -1 && ComputeBalance(root->Right) > 0)
        {
            root->Right = RotateRight(root->Right);
            return RotateLeft(root);
        }

        return root;
    }

    [[nodiscard]] static Node* Remove(Node** const rootHolder, const Node* const search) noexcept
    {
        if(!rootHolder || !(*rootHolder))
        {
            return nullptr;
        }

        Node* root = *rootHolder;

        if(search->Value < root->Value)
        {
            root->Left = Remove(&root->Left, search);
        }
        else if(search->Value > root->Value)
        {
            root->Right = Remove(&root->Right, search);
        }
        else if(search != root)
        {
            if constexpr(InsertMethod == InsertMethod::Ignore || InsertMethod == InsertMethod::Replace)
            {
                return root;
            }
            else if constexpr(InsertMethod == InsertMethod::Greater)
            {
                root->Right = Remove(&root->Right, search);
            }
            else if constexpr(InsertMethod == InsertMethod::Lesser)
            {
                root->Left = Remove(&root->Left, search);
            }
        }
        else
        {
            if(!root->Left || !root->Right)
            {
                Node* tmp = root->Left ? root->Left : root->Right;
                if(!tmp)
                {
                    *rootHolder = nullptr;
                }
                else
                {
                    *rootHolder = tmp;
                }

                delete root;
                root = *rootHolder;
            }
            else
            {
                Node** tmp = MinValueNode(&root->Right);

                *rootHolder = *tmp;                 // Replace root
                *tmp = (*tmp)->Right;               // Store tmp's right branch in tmp's parent left branch
                (*rootHolder)->Left = root->Left;   // Set tmp's left branch to the old root's left branch
                (*rootHolder)->Right = root->Right; // Set tmp's right branch to the old root's right branch
                delete root;                        // Destroy root 
                root = *rootHolder;                 // Update the actual root variable
            }
        }

        if(!root)
        { return nullptr; }

        root->Height = maxT(Height(root->Left), Height(root->Right)) + 1;

        const HeightT balance = ComputeBalance(root);

        // Left Left
        if(balance > 1 && ComputeBalance(root->Left) >= 0)
        {
            return RotateRight(root);
        }

        // Left Right
        if(balance > 1 && ComputeBalance(root->Left) < 0)
        {
            root->Left = RotateLeft(root->Left);
            return RotateRight(root);
        }

        // Right Right
        if(balance < -1 && ComputeBalance(root->Right) <= 0)
        {
            return RotateLeft(root);
        }

        // Right Left
        if(balance < -1 && ComputeBalance(root->Right) > 0)
        {
            root->Right = RotateRight(root->Right);
            return RotateLeft(root);
        }

        return root;
    }

    static void DisposeTree(Node* const tree) noexcept
    {
        if(!tree)
        {
            return;
        }

        DisposeTree(tree->Left);
        DisposeTree(tree->Right);

        delete tree;
    }

    template<typename NodeT, typename FuncT, IteratorMethod IteratorMethod>
    static void IterateStatic(NodeT* const tree, const FuncT& func) noexcept
    {
        if(!tree)
        {
            return;
        }

        if constexpr(IteratorMethod == IteratorMethod::TopDownLR)
        {
            func(tree);
            IterateStatic<NodeT, FuncT, IteratorMethod>(tree->Left, func);
            IterateStatic<NodeT, FuncT, IteratorMethod>(tree->Right, func);
        }
        else if constexpr(IteratorMethod == IteratorMethod::TopDownRL)
        {
            func(tree);
            IterateStatic<NodeT, FuncT, IteratorMethod>(tree->Right, func);
            IterateStatic<NodeT, FuncT, IteratorMethod>(tree->Left, func);
        }
        else if constexpr(IteratorMethod == IteratorMethod::HighestToLowest)
        {
            IterateStatic<NodeT, FuncT, IteratorMethod>(tree->Right, func);
            IterateStatic<NodeT, FuncT, IteratorMethod>(tree->Left, func);
            func(tree);
        }
        else if constexpr(IteratorMethod == IteratorMethod::LowestToHighest)
        {
            IterateStatic<NodeT, FuncT, IteratorMethod>(tree->Left, func);
            IterateStatic<NodeT, FuncT, IteratorMethod>(tree->Right, func);
            func(tree);
        }
    }

    template<typename NodeT, typename ClassC, typename FuncT, IteratorMethod IteratorMethod>
    static void IterateStatic(NodeT* const tree, ClassC* const instance, const FuncT& func) noexcept
    {
        if(!tree)
        {
            return;
        }

        if constexpr(IteratorMethod == IteratorMethod::TopDownLR)
        {
            (instance->*func)(tree);
            IterateStatic<NodeT, ClassC, FuncT, IteratorMethod>(tree->Left, instance, func);
            IterateStatic<NodeT, ClassC, FuncT, IteratorMethod>(tree->Right, instance, func);
        }
        else if constexpr(IteratorMethod == IteratorMethod::TopDownRL)
        {
            (instance->*func)(tree);
            IterateStatic<NodeT, ClassC, FuncT, IteratorMethod>(tree->Right, instance, func);
            IterateStatic<NodeT, ClassC, FuncT, IteratorMethod>(tree->Left, instance, func);
        }
        else if constexpr(IteratorMethod == IteratorMethod::HighestToLowest)
        {
            IterateStatic<NodeT, ClassC, FuncT, IteratorMethod>(tree->Right, instance, func);
            IterateStatic<NodeT, ClassC, FuncT, IteratorMethod>(tree->Left, instance, func);
            (instance->*func)(tree);
        }
        else if constexpr(IteratorMethod == IteratorMethod::LowestToHighest)
        {
            IterateStatic<NodeT, ClassC, FuncT, IteratorMethod>(tree->Left, instance, func);
            IterateStatic<NodeT, ClassC, FuncT, IteratorMethod>(tree->Right, instance, func);
            (instance->*func)(tree);
        }
    }
};

template<typename T>
using PackedAVLNode = AVLNode<T, int8_t>;

template<typename T>
using FastAVLNode = AVLNode<T, intptr_t>;

template<typename T, InsertMethod InsertMethod = InsertMethod::Ignore>
using PackedAVLTree = AVLTree<T, int8_t, InsertMethod>;

template<typename T, InsertMethod InsertMethod = InsertMethod::Ignore>
using FastAVLTree = AVLTree<T, intptr_t, InsertMethod>;
