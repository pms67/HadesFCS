using System.Collections;
using NUnit.Framework;
using UnityEngine.TestTools;
using UnityEngine;
using UnityEngine.Networking;

#pragma warning disable 618
public class LocalClientSpawnsObjectAfterConnect : SpawningTestBase
{
    bool isDone;
    GameObject obj;

    [UnityTest]
    public IEnumerator LocalClientSpawnsObjectAfterConnectTest()
    {
        NetworkClient.ShutdownAll();
        NetworkServer.Reset();

        SetupPrefabs();
        StartServer();
        StartLocalClient();

        while (!isDone)
        {
            yield return null;
        }

        // 2 is player and rock
        Assert.AreEqual(2, numStartServer);
        Assert.AreEqual(2, numStartClient);

        ClientScene.DestroyAllClientObjects();
        yield return null;
        NetworkServer.Destroy(obj);
        NetworkServer.Destroy(playerObj);
    }

    public override void OnServerReady(GameObject player)
    {
        obj = (GameObject)Instantiate(rockPrefab, Vector3.zero, Quaternion.identity);
        NetworkServer.Spawn(obj);
        isDone = true;
    }
}
#pragma warning restore 618
