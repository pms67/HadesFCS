using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Text;
using System.IO;

public class CSVLogger {
    private StringBuilder sb;

    public CSVLogger()
    {
        
    }

    public void New(string[] columns)
    {
        sb = new StringBuilder();
        sb.AppendLine(string.Join(",", columns));
    }

    public void AddLine(string[] data)
    {
        sb.AppendLine(string.Join(",", data));
    }

    public void WriteFile(string filename)
    {
        File.WriteAllText(filename, sb.ToString());
    }
}
