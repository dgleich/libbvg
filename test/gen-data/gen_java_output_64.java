import it.unimi.dsi.io.OutputBitStream;

import java.io.*;

public class gen_java_output_64
{
    public static void main(String[] args) throws
                ClassNotFoundException, IOException
    {
        OutputStream os = new FileOutputStream("correct-data-64");
        OutputBitStream obs = new OutputBitStream(os);

        obs.writeInt(13,10);
        obs.writeInt(4,3);
        obs.writeInt(3,3);
        obs.writeInt(45233,16);
        obs.writeInt(232584213,30);
        for (int i = 0; i < 64; i++) {
            obs.writeUnary(i);
        }

        for (int i = 0; i < 64; i++) {
            obs.writeGamma(7*i+i);
        }

        for (int i = 0; i < 64; i++) {
            obs.writeZeta(7*i+i,3);
        }
        
        obs.writeLong(4294967296l,35);

        for (long i = 0; i < 16; i++) {
            obs.writeLongGamma(4294967296l*(1l << i));
        }
        
        for (long i = 0; i < 16; i++) {
            obs.writeLongZeta(4294967296l*(1 << i), 3);
        }

        obs.flush();
        obs.close();
        os.flush();
        os.close();
    
    }

}
