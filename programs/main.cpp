#include <vector>
#include <iostream>



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



/////



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

   class ActionFactory
   {
   public:
      static ActionInterface* decrement_integer(Integer *integer) { return new Actions::Increment(integer); }
      static ActionInterface* increment_integer(Integer *integer) { return new Actions::Decrement(integer); }
   };
}



/////



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


/////


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


/////


int main(int, char**)
{
   App app;
   app.run_loop();

   return 0;
}


