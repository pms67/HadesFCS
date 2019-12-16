using System.Collections;
using NUnit.Framework;
using UnityEngine.TestTools;
using UnityEngine;
using UnityEngine.Networking;

#pragma warning disable 618
public class SpawnObjectOnServerOnly : SpawningTestBase
{
    GameObject obj;

    [UnityTest]
    public IEnumerator SpawnObjectOnServerOnlyTest()
    {
        NetworkClient.ShutdownAll();
        NetworkServer.Reset();

        SetupPrefabs();
        StartServer();

        obj = (GameObject)Instantiate(rockPrefab, Vector3.zero, Quaternion.identity);
        NetworkServer.Spawn(obj);
        yield return null;

        // 1 is rock, there is no player
        Assert.AreEqual(1, numStartServer);

        NetworkServer.Destroy(obj);
    }
}
#pragma warning restore 618
