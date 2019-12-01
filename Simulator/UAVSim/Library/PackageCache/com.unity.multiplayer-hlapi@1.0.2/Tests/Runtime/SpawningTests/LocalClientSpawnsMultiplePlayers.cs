using System.Collections;
using System.Collections.Generic;
using NUnit.Framework;
using UnityEngine.TestTools;
using UnityEngine;
using UnityEngine.Networking;

#pragma warning disable 618
public class LocalClientSpawnsMultiplePlayers : SpawningTestBase
{
    const int kPlayerCount = 2;
    List<short> m_ReadyPlayers = new List<short>();
    private int numPlayers = 0;
    GameObject obj;

    [UnityTest]
    public IEnumerator LocalClientSpawnsMultiplePlayersTest()
    {
        NetworkClient.ShutdownAll();
        NetworkServer.Reset();

        SetupPrefabs();
        StartServer();
        StartLocalClient(kPlayerCount);

        while (m_ReadyPlayers.Count != kPlayerCount)
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

        // rock + this player
        Assert.AreEqual(numPlayers + 2, numStartServer);
        Assert.AreEqual(numPlayers + 2, numStartClient);

        numPlayers += 2;
    }

    public override void OnClientReady(short playerId)
    {
        // Sanity check. Make sure these are unique player IDs each time
        if (!m_ReadyPlayers.Contains(playerId))
        {
            m_ReadyPlayers.Add(playerId);
        }
        else
        {
            Assert.Fail("Player with such Id already exist");
        }
    }
}
#pragma warning restore 618
