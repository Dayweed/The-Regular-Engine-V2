using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using GlmSharp;

namespace TRE 
{
    public class PauseMenu : Entity
    {
        public bool isPaused = true;
        public bool isConfirming = false;

        public int mainMenuOption = 0; // for confirmation menu to know which option to execute
        public int cfmMenuOption = 0; // for main update logic to know what to execute with destructive action
        public int currentOption = 0;
        public int menustate = 0; //0 = main menu, 1 = options menu, 2 = confirmation menu
        public int previousMenuState = 0;


        // Pause menu pointer
        private Entity pointer;
        private Transform pointerTransform;

        // Pause menu options
        private List<Entity> options;

        private List<Entity> DestructiveActionConfirmations;

        private Entity Menu1;
        private Entity cfmMenu;



        public void Start()
        {
            pointer = ECSManager.FindEntityByName("pauseMenu_Pointer");
            pointerTransform = pointer.GetComponent<Transform>();
            options.Add(ECSManager.FindEntityByName("pauseMenu_Resume"));
            options.Add(ECSManager.FindEntityByName("pauseMenu_Options"));
            options.Add(ECSManager.FindEntityByName("pauseMenu_return"));
            options.Add(ECSManager.FindEntityByName("pauseMenu_Quit"));

            DestructiveActionConfirmations.Add(ECSManager.FindEntityByName("pauseMenu_cfmYes"));
            DestructiveActionConfirmations.Add(ECSManager.FindEntityByName("pauseMenu_cfmNo"));

            Menu1 = ECSManager.FindEntityByName("PauseMenu");
            cfmMenu = ECSManager.FindEntityByName("pauseMenu_cfmMenu");
        }

        public void OnCreate()
        {

        }

        public void Update()
        {

            // pointer Handler
            vec3 position = new vec3(0, 0, 0);
            position.x = pointerTransform.Position.x;


            // Pause menu logic
            if (menustate == 0)
            {
                //we are assuming that pause menu entering is handled by game logic
                if (InputSystem.GetKeyDown(InputKeys.W))
                {
                    if (currentOption == 0)
                        currentOption -= 1;
                    else if (currentOption == 3)
                        currentOption = 0;
                    else
                        currentOption += 1;
                }

                if (InputSystem.GetKeyDown(InputKeys.S))
                {
                    if (currentOption == 0)
                        currentOption += 1;
                    else if (currentOption == 3)
                        currentOption = 0;
                    else
                        currentOption -= 1;
                }

                if (InputSystem.GetKeyDown(InputKeys.Enter))
                {
                    if (currentOption == 0) // resume game
                    {
                        isPaused = false;
                        menustate = 0;
                    }
                    else if (currentOption == 1) // options
                    {
                        menustate = 1;
                    }
                    else if (currentOption == 2) // return to title
                    {
                        isConfirming = true;
                        mainMenuOption = 2;
                        menustate = 2;
                    }
                    else if (currentOption == 3) // quit game
                    {
                        //quit game
                        isConfirming = true;
                        mainMenuOption = 3;
                        menustate = 2;
                    }
                }

                if (InputSystem.GetKeyDown(InputKeys.Escape))
                {
                    isPaused = false;
                    menustate = 0;
                }   
                
                switch (currentOption)
                {
                    case 0: // resume
                        position.y = options[0].GetComponent<Transform>().Position.y;
                        break;
                    case 1: // 
                        position.y = options[1].GetComponent<Transform>().Position.y;
                        break;
                    case 2:
                        position.y = options[2].GetComponent<Transform>().Position.y;
                        break;
                    case 3:
                        position.y = options[3].GetComponent<Transform>().Position.y;
                        break;
                }
            }
            
            // options menu logic
            if(menustate == 1)
            {

            }

            // confirmation menu logic
            if (menustate == 2)
            {
                if (InputSystem.GetKeyDown(InputKeys.W))
                {
                    if (currentOption == 0)
                        currentOption -= 1;
                    else if (currentOption == 1)
                        currentOption = 0;
                }

                if (InputSystem.GetKeyDown(InputKeys.S))
                {
                    if (currentOption == 0)
                        currentOption += 1;
                    else if (currentOption == 1)
                        currentOption = 0;
                }

                switch (currentOption)
                {
                    case 0: // yes
                        position.y = DestructiveActionConfirmations[0].GetComponent<Transform>().Position.y;
                        break;
                    case 1: // no
                        position.y = DestructiveActionConfirmations[1].GetComponent<Transform>().Position.y;
                        break;
                }

            }


            if (previousMenuState != menustate)
            {
                switch (menustate)
                {
                    case 0:
                        
                        break;
                    case 1:
                        
                        break;
                    case 2:
                        
                        break;
                }
            }

        }
    }
}
