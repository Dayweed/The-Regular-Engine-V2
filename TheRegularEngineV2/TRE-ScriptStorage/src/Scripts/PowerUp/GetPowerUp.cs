using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;
using GlmSharp;

namespace TRE
{
	public class GetPowerUp : Entity
	{
		public PowerUpsType powerUpType;
		private Entity PowerUpManagerObj;

		private string mole1tag = "Red";
		private string mole2tag = "Blue";

		public bool collected = false;

		// VFX variables
		private bool vfxCollectComplete = false;
		private float vfxLerpSpeed = 1.5f;
		private vec3 originalScale;

		public float cooldownDuration = 1.5f;
		public float cooldownCurrent = 0f;

		//private Renderer headPiece;                   // THIS CANT BE DONE YET!
		private Entity playerObj;                       // private Transform playerObj;
														// private Entity playerModel;                  // private Transform playerModel;

		private PowerUpManager playerPowerUpManager;

		private float groundOffset = 2;
		private float rotationSpeed = 20;

		private ulong collectedSFX;

		public GetPowerUp()
		{

		}

		public void Start()
		{
			collectedSFX = ECSManager.FindIDFromName("SFX_PowerUpsCollected");
			originalScale = GetComponent<Transform>().Scale;
        }

		private void OnTriggerStay(/*Collider*/System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			//Debug.Log("Triggered with " + ECSManager.FindNameFromID(other.ID));

			if (GetComponent<Rigidbody>().useGravity == true && other.CompareTag("Ground"))
			{
				//RemoveComponent<Rigidbody>();
				GetComponent<Rigidbody>().useGravity = false;
				PhysicsSystem.SetLinearVelocity(ID, vec3.Zero);
				transform.Position = new vec3(transform.Position.x, transform.Position.y + transform.Scale.y + groundOffset, transform.Position.z);
				//PhysicsSystem.SetLinearVelocity(ID, Vector3.zero);
				cooldownCurrent = 0;
				return;
			}

			if (collected || cooldownCurrent > 0) return;

			if (other.CompareTag(mole1tag) || other.CompareTag(mole2tag))
			{
				//headPiece = other.GetComponent<Renderer>();                           // THIS CANT BE DONE YET!
				//playerModel = other.parenting.GetParent();                              // playerModel = other.transform.parent;
				//playerObj = playerModel.parenting.GetParent();                          // playerObj = playerModel.parent;

				playerObj = other;
				PowerUpManagerObj = playerObj.parenting.GetChildFromName("Power Manager");

				if (!ECSManager.IsValidEntity(PowerUpManagerObj.ID))
				{
					Debug.LogError("Could not find PowerUpManagerObj (" + PowerUpManagerObj.ID + ")");
					return;
				}
				playerPowerUpManager = PowerUpManagerObj.GetComponent<PowerUpManager>();        //playerPowerUpManager = playerObj.GetComponent<PowerUpManager>();

				if (playerPowerUpManager == null)
				{
					Debug.LogError("Could not find playerPowerUpManager");
					return;
				}
				if (playerPowerUpManager.powerUps == null)
				{
					Debug.LogError("Could not find playerPowerUpManager.powerUps");
					return;
				}

				//if player already has 2 power-ups, don't pick up a 3rd one
				if (playerPowerUpManager.powerUps.Count == 2) return;

				//Debug.Log("Collided with " + ECSManager.FindNameFromID(other.ID));
				playerPowerUpManager.powerUps.Add(this);                            // playerPowerUpManager.powerUps.Add(this.gameObject);
				playerPowerUpManager.MyPowerUpUI.UpdateUI(playerPowerUpManager.powerUps);

				SetToPlayer();

				collected = true;

				if (collected && ECSManager.IsValidEntity(collectedSFX))
				{
					AudioSystem.Play(collectedSFX);
				}

				GetComponent<Rigidbody>().useGravity = false;
				PhysicsSystem.SetLinearVelocity(ID, vec3.Zero);

				vfxCollectComplete = false;
                RunCollectingVFX();
            }
		}

		private void OnCollisionStay(System.UInt64 otherID)
		{
			Entity other = new Entity(otherID);
			//Debug.Log("Collided with " + ECSManager.FindNameFromID(other.ID));
		}

		public void Update()
		{
			// Run Collecting vfx
			if (collected && !vfxCollectComplete)
			{
				RunCollectingVFX();
            }
			if (!collected)
			{
				// Spin blueberry
				float newRot = transform.Rotation.y + rotationSpeed * Time.deltaTime;
				transform.Rotation = new vec3(0, newRot, 0);
				return;
			}
			if (cooldownCurrent >= 0) cooldownCurrent -= Time.deltaTime;
			SetToPlayer();
		}

		private void RunCollectingVFX()
		{
			Transform myTransform = GetComponent<Transform>();
            float x = MathF.Lerp(myTransform.Scale.x, 0, vfxLerpSpeed * Time.deltaTime);
            float y = MathF.Lerp(myTransform.Scale.y, 0, vfxLerpSpeed * Time.deltaTime);
            float z = MathF.Lerp(myTransform.Scale.z, 0, vfxLerpSpeed * Time.deltaTime);
			myTransform.Scale = new vec3(x, y, z);
			// Deactivate Mesh if < 0
			if (x <= 0 || y <= 0 || z <= 0)
			{
				GetComponent<MeshRenderer>().Visible = false;

                // Reset size
                GetComponent<Transform>().Scale = originalScale;

				vfxCollectComplete = true;
            }
        }

		private void SetToPlayer()
		{
			//move the power up gameobj to the player's position
			//parenting.SetParent(playerObj);                                     // this.transform.parent = playerObj;

			//this.transform.Position =  new Vector3(0, this.transform.Position.y, 0);     // this.transform.localPosition = new Vector3(0, this.transform.localPosition.y, 0);

			if (!collected || playerObj == null || ECSManager.IsValidEntity(playerObj.ID) == false) return;

			//GetComponent<MeshRenderer>().Visible = false;

			vec3 newPos = playerObj.transform.Position;
			int collectedIndex = playerPowerUpManager.powerUps.IndexOf(this) + 1;
			//newPos.y += playerObj.transform.Scale.y * 4 + (transform.Scale.y * 4 * collectedIndex - 1);
			if (playerObj.CompareTag(mole1tag))
			{
				newPos.y += 6f;// playerObj.GetComponent<MoleyController>().currentHeight * 1.5f + 5f;
			}

			if (playerObj.CompareTag(mole2tag))
			{
				newPos.y += 6f;// playerObj.GetComponent<HoleyController>().currentHeight * 1.5f + 5f;
			}
			transform.Position = newPos;
			//transform.Position = newPos;

			//RigidBodySystem.SetKinematic(ID, false);                             //this.gameObject.GetComponent<Rigidbody>().isKinematic = true;     // THIS CANT BE DONE YET!
			//this.gameObject.GetComponent<Collider>().enabled = false;         // THIS CANT BE DONE YET!
			//this.gameObject.GetComponent<RotateObj>().enabled = false;        // THIS CANT BE DONE YET!
			//SetActive(false);                                                   //this.transform.GetChild(1).gameObject.SetActive(false);
		}

		public void ReleasePowerUp()
		{
            // Reset size
            GetComponent<Transform>().Scale = originalScale;
            GetComponent<MeshRenderer>().Visible = true;

            // Get player facing direction
            vec2 dir = new vec2(0, -1);
            float angle = -playerObj.GetComponent<Transform>().Rotation.y;
            dir.x = (float)(Math.Cos(angle) * dir.x - Math.Sin(angle) * dir.y);
            dir.y = (float)(Math.Sin(angle) * dir.x + Math.Cos(angle) * dir.y);
            dir = dir.Normalized;

            playerObj = null;
			collected = false;
			GetComponent<Rigidbody>().useGravity = true;
			PhysicsSystem.AddForce(this.ID, new vec3(dir.x * 10, 50, dir.y * 10), ForceMode.VelocityChange);
			cooldownCurrent = cooldownDuration;
		}

		public void TurnOnVisuals()
		{
			// No clue what this does yet
			//parenting.RemoveParent();                                           //this.transform.parent = null;
			//parenting.GetChild(1).SetActive(true);                              //this.transform.GetChild(1).gameObject.SetActive(true);

			//TurnOnCollider();//Invoke(nameof(TurnOnCollider), 0.5f);            // THIS CANT BE DONE YET!
		}

		public void TurnOnCollider()
		{
			//this.gameObject.GetComponent<Collider>().enabled = true;          // THIS CANT BE DONE YET!
			//this.gameObject.GetComponent<RotateObj>().enabled = true;         // THIS CANT BE DONE YET!
		}
	}



	public enum PowerUpsType { Blueberry, Strawberry }
}