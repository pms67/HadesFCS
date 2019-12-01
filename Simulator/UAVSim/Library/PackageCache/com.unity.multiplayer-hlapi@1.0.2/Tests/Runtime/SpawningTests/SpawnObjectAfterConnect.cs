using System.Collections;
using NUnit.Framework;
using UnityEngine.TestTools;
using UnityEngine;
using UnityEngine.Networking;

#pragma warning disable 618
public class SpawnObjectAfterConnect : SpawningTestBase
{
    bool isDone;
    GameObject obj;

    [UnityTest]
    public IEnumerator SpawnObjectAfterConnectTest()
    {
        NetworkClient.ShutdownAll();
        NetworkServer.Reset();

        SetupPrefabs();
        StartServer();
        NetworkServer.SpawnObjects();
        StartClientAndConnect();

        while (!isDone)
        {
            yield return null;
        }
        ClientScene.DestroyAllClientObjects();
        yield return null;
        NetworkServer.Destroy(obj);
        NetworkServer.Destroy(playerObj);
    }

    public override void OnServerReady(GameObject player)
    {
        obj = (GameObject)Instantiate(rockPrefab, Vector3.zero, Quaternion.identity);
        NetworkServer.Spawn(obj);

        Assert.AreEqual(2, numStartServer);
    }

    public override void OnClientReady(short playerId)
    {
        Assert.AreEqual(2, numStartClient);
        isDone = true;
    }
}
#pragma warning restore 618
