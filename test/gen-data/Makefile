
all : dsiutils-2.2.0.jar fastutil-6.5.15.jar gen_java_output.class gen_java_output_64.class correct-data correct-data-64

correct-data : gen_java_output.class fastutil-6.5.15.jar
	java -classpath dsiutils-2.2.0.jar:fastutil-6.5.15.jar:. gen_java_output

correct-data-64 : gen_java_output.class fastutil-6.5.15.jar
	java -classpath dsiutils-2.2.0.jar:fastutil-6.5.15.jar:. gen_java_output_64


gen_java_output.class : gen_java_output.java dsiutils-2.2.0.jar
	javac -classpath dsiutils-2.2.0.jar gen_java_output.java
    
gen_java_output_64.class : gen_java_output_64.java dsiutils-2.2.0.jar
	javac -classpath dsiutils-2.2.0.jar gen_java_output_64.java    

dsiutils-2.2.0.jar :
	wget http://search.maven.org/remotecontent?filepath=it/unimi/dsi/dsiutils/2.2.0/dsiutils-2.2.0.jar -O dsiutils-2.2.0.jar

fastutil-6.5.15.jar :
	wget http://search.maven.org/remotecontent?filepath=it/unimi/dsi/fastutil/6.5.15/fastutil-6.5.15.jar -O fastutil-6.5.15.jar 
