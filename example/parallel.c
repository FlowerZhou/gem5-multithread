#include <iostream>
#include <thread>

using namespace std;

void thread01()
{
  for (int i = 0; i < 1000; i++)
  {
      cout << "Thread 01 is working ！" << endl;
      this_thread::sleep_for( chrono::seconds(1) );  
  }
}
void thread02()
{
  for (int i = 0; i < 1000; i++)
  {
      cout << "Thread 02 is working ！" << endl;
      this_thread::sleep_for( chrono::seconds(1) );
      
    }
}

int main()
{
  thread task01(thread01);
  thread task02(thread02);
  task01.join();
  task02.join();

  for (int i = 0; i < 10; i++)
  {
      cout << "Main thread is working ！" << endl;
      this_thread::sleep_for( chrono::seconds(1) );  
  }
  return 0;
}
