# coding=utf-8
class Student:
    def __new__(cls, *args, **kwargs):
        print("执行__new__",args[0],args[1])
        return object.__new__(cls) 
    def __init__(self,name,age):
        print("执行__init__",name,age)
        self.name=name
        self.age=age
    def showName(self):
        print("姓名：{}".format(self.name))
zhangsan=Student("张三",22)
zhangsan.showName()

