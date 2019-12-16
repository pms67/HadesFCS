using NUnit.Framework;
using System;
using System.Collections;
using UnityEngine;
using UnityEngine.TestTools;
using UnityEngine.Networking;
using UnityEngine.Networking.NetworkSystem;

#pragma warning disable 618
public class NetworkMigrationWorksTest
{
    private GameObject netManagerObj;
    NetworkManager networkManager;
    NetworkMigrationManager networkMigrManager;
    private NetworkClient client;
    int _port = 8888;
    string _ip = "127.0.0.1";

    [UnityTest]
    public IEnumerator NetworkMigrationWorksCheck()
    {
        NetworkServer.Reset();

        SetupNetwork();

        yield return new WaitUntil(() => networkMigrManager.peers != null);

        networkManager.StopServer();

        PeerInfoMessage newHostInfo;
        bool youAreNewHost;
        Assert.IsTrue(
            networkMigrManager.FindNewHost(out newHostInfo, out youAreNewHost),
            "New host was not found.");

        Assert.IsTrue(
            client.ReconnectToNewHost(newHostInfo.address, newHostInfo.port),
            "Old client did not reconnect to new host.");

        yield return null;
        UnityEngine.Object.Destroy(netManagerObj);
    }

    public void SetupNetwork()
    {
        netManagerObj = new GameObject();
        networkManager = netManagerObj.AddComponent<NetworkManager>();
        networkManager.playerPrefab =  Resources.Load("PlayerGameObject", typeof(GameObject)) as GameObject;

        Assert.IsNotNull(networkManager.playerPrefab);

        networkManager.customConfig = true;
        networkManager.networkAddress = _ip;
        networkManager.networkPort = _port;
        networkManager.autoCreatePlayer = false;

        networkMigrManager = netManagerObj.AddComponent<NetworkMigrationManager>();
        Assert.IsTrue(networkManager.StartServer(), "Server was not started!");
        networkManager.SetupMigrationManager(networkMigrManager);

        client = networkManager.StartClient();
        client.Connect(_ip, _port);
        Assert.IsNull(client.connection, "Client is not connected");

        networkMigrManager.Initialize(client, networkManager.matchInfo);
        networkMigrManager.SendPeerInfo();
    }
}
#pragma warning restore 618
