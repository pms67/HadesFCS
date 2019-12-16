using System.Collections;
using NUnit.Framework;
using UnityEngine.TestTools;
using UnityEngine;
using UnityEngine.Networking;

#pragma warning disable 618
public class SpawnObjectBeforeConnect : SpawningTestBase
{
    bool isDone;

    [UnityTest]
    public IEnumerator SpawnObjectBeforeConnectTest()
    {
        NetworkClient.ShutdownAll();
        NetworkServer.Reset();

        SetupPrefabs();
        StartServer();

        GameObject obj = (GameObject)Instantiate(rockPrefab, Vector3.zero, Quaternion.identity);
        NetworkServer.Spawn(obj);
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
        Assert.AreEqual(2, numStartServer);
    }

    public override void OnClientReady(short playerId)
    {
        Assert.AreEqual(2, numStartClient);
        isDone = true;
    }
}
#pragma warning restore 618
