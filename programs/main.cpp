///// Layer 1: The Language and its Libraries


#include <vector>
#include <iostream>



///// Layer 2: Framework Layer



bool debug = false;



class ActionInterface
{
private:
   std::string name;

public:
   ActionInterface(std::string name) : name(name) {}
   std::string get_name() { return name; }
   virtual ~ActionInterface() {}
   virtual void execute() = 0;
};



class ActionQueue : public ActionInterface
{
public:
   std::vector<ActionInterface *> actions;
   ActionQueue() : ActionInterface("ActionQueue"), actions() {}; 

   void append(ActionInterface *action) { actions.push_back(action); }
   virtual void execute() override
   {
      for (auto &action : actions) { action->execute(); }
      for (auto &action : actions) { delete action; }
      actions.clear();
   }
};



//////////////////////
//////////////////////
//////////////////////



///// Layer 3: Model Layer - contains primitave classes and simple functions to modify them



namespace Integer
{
   class Integer
   {
   public:
      int i;

      Integer(int i) : i(i) {}
      void increment() { i++; }
      void decrement() { i--; }
   };
}



namespace WebAPIRequester
{
   class WebAPIRequester
   {
   public:
      std::string url;

      WebAPIRequester(std::string url) : url(url) {}
      std::string response() { /* call url and get response */ return "{ \"response\": \"hello!\" }"; }
   };
}



namespace Scene // Scene State layer (same layer height as a model layer, but contains the state, objects, etc in the scene)
{
   class Scene
   {
   public:
      Integer::Integer integer_a;
      Integer::Integer integer_b;

      //Widget widget_a;
      //Widget widget_b;

      Scene()
         : integer_a(10)
         , integer_b(100)
      {}
   };
}



///// Layer 4: Model Action layer



namespace Integer
{
   namespace Actions
   {
      class Increment : public ActionInterface
      {
      private:
         Integer *klass;

      public:
         Increment(Integer *klass) : ActionInterface("Increment"), klass(klass){}
         virtual void execute() override { klass->increment(); }
      };

      class Decrement : public ActionInterface
      {
      private:
         Integer *klass;

      public:
         Decrement(Integer *klass) : ActionInterface("Decrement"), klass(klass) {}
         virtual void execute() override { klass->decrement(); }
      };
   }
}



namespace Scene
{
   namespace Actions
   {
      class IncrementAllIntegers : public ActionInterface
      {
      private:
         Scene *scene;

      public:
         IncrementAllIntegers(Scene *scene) : ActionInterface("IncrementAllIntegers"), scene(scene)
         {}
         virtual void execute() override
         {
            scene->integer_a.increment();
            scene->integer_b.increment();
         }
      };
   };
}



///// Layer 5: Action Factory Layer - contains factory functions that act on the underlying models



class ActionFactory
{
public:
   static ActionInterface* decrement_integer(Integer::Integer *integer) { return new Integer::Actions::Increment(integer); }
   static ActionInterface* increment_integer(Integer::Integer *integer) { return new Integer::Actions::Decrement(integer); }
   static ActionInterface* increment_all_integers(Scene::Scene *scene) { return new Scene::Actions::IncrementAllIntegers(scene); }
};



//////////////////////
//////////////////////
//////////////////////



///// Layer 6: User Interface Layer - manages interactions from input and the actions that can be executed by the imputs



namespace UserInterface
{
   class UserInterface
   {
   public:
      ActionQueue action_queue;
      bool program_abort;

      UserInterface()
         : action_queue()
         , program_abort(false)
      {}
      void abort_program() { program_abort = true; }
      bool is_interface_ready_to_exit() { return program_abort; }
      void emit_action(ActionInterface *action)
      {
         if (debug) std::cout << action->get_name() << std::endl;
         action_queue.append(action);
      }
   };

   class ActionFactory
   {
   public:
      static ActionInterface* process_input(UserInterface *user_interface, char input_char);
      static ActionInterface* say_hello();
      static ActionInterface* abort_program(UserInterface *user_interface);
   };

   namespace Actions
   {
      class AbortProgram : public ActionInterface
      {
      private:
         UserInterface *klass;

      public:
         AbortProgram(UserInterface *klass) : ActionInterface("AbortProgram"), klass(klass) {}
         virtual void execute() override { klass->abort_program(); }
      };

      class SayHello : public ActionInterface
      {
      public:
         SayHello() : ActionInterface("SayHello") {}
         virtual void execute() override { std::cout << "Hello World!" << std::endl; }
      };

      class ProcessInput : public ActionInterface
      {
      private:
         UserInterface *user_interface;
         char input;

      public:
         ProcessInput(UserInterface *user_interface, char input)
            : ActionInterface("ProcessInput")
            , user_interface(user_interface)
            , input(input)
         {}
         virtual void execute() override
         {
            switch (input)
            {
            case 'h': user_interface->emit_action(ActionFactory::say_hello()); break;
            case 'q': user_interface->emit_action(ActionFactory::abort_program(user_interface)); break;
            }
         }
      };
   }

   ActionInterface* ActionFactory::process_input(UserInterface *user_interface, char input_char) { return new Actions::ProcessInput(user_interface, input_char); }
   ActionInterface* ActionFactory::say_hello() { return new Actions::SayHello(); }
   ActionInterface* ActionFactory::abort_program(UserInterface *user_interface) { return new Actions::AbortProgram(user_interface); }
}



///// Layer 7: App Layer - feeds input from the system into the user interface layer



class App
{
public:
   ActionQueue action_queue;
   UserInterface::UserInterface user_interface;

   App()
     : action_queue()
     , user_interface()
   {}

   void run_loop()
   {
      while (!user_interface.is_interface_ready_to_exit())
      {
         char input_char;
         std::cin >> input_char;
         ActionInterface *input = UserInterface::ActionFactory::process_input(&user_interface, input_char);
         input->execute();

         while (!user_interface.action_queue.actions.empty())
         {
            if (debug) std::cout << "Executing action: " << user_interface.action_queue.actions[0]->get_name() << std::endl;
            user_interface.action_queue.actions[0]->execute();
            delete user_interface.action_queue.actions[0];
            user_interface.action_queue.actions.erase(user_interface.action_queue.actions.begin());
         }
      }
   }
};



///// Layer X: Executable Layer



int main(int, char**)
{
   App app;
   app.run_loop();

   return 0;
}



