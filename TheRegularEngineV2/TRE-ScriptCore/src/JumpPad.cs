using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.RuntimeHelpers;
using System.Threading;

namespace TRE
{
    public class JumpPad : Entity
    {
        public Entity entitySwitch;
        public bool isActivated;

        private PowerUpManager playerPowerUpManager;

        public JumpPad()
        {

        }

        public void OnCreate()
        {
            isActivated = true;
        }

        public void Update()
        {

        }
    }
}