#pragma once

class Scene
{
private:
   std::vector<std::unique_ptr<Node>> Nodes;

public:
   Scene() = default;
   ~Scene() = default;

   void AddNode(std::unique_ptr<Node> node)
   {
      Nodes.push_back(std::move(node));
   }

   bool RemoveNode(Node* node)
   {
      auto it = std::ranges::find_if(Nodes, [node](const std::unique_ptr<Node> &n) { return n.get() == node; });
      if (it != Nodes.end())
      {
         Nodes.erase(it);
         return true;
      }
      return false;
   }

   const std::vector<std::unique_ptr<Node>>& GetNodes() const { return Nodes; }

   template<typename T>
      requires std::derived_from<T, Node>
   std::vector<T*> GetNodesOfType() const
   {
      std::vector<T*> nodes;
      for (const auto& node : Nodes)
      {
         if (T* t = dynamic_cast<T*>(node.get()))
         {
            nodes.push_back(t);
         }
      }
      return nodes;
   }
};
