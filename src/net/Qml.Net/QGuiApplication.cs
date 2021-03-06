﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using AdvancedDLSupport;
using Qml.Net.Internal;
using Qml.Net.Internal.Qml;

namespace Qml.Net
{
    public sealed class QGuiApplication : BaseDisposable
    {
        readonly Queue<Action> _actionQueue = new Queue<Action>();
        GCHandle _triggerHandle;
        readonly SynchronizationContext _oldSynchronizationContext;

        public QGuiApplication()
            :this(null)
        {
            
        }
        
        public QGuiApplication(string[] args)
            :base(Create(args?.ToList()))
        {
            TriggerDelegate triggerDelegate = Trigger;
            _triggerHandle = GCHandle.Alloc(triggerDelegate);
            
            Interop.QGuiApplication.AddTriggerCallback(Handle, Marshal.GetFunctionPointerForDelegate(triggerDelegate));
            
            _oldSynchronizationContext = SynchronizationContext.Current;
            SynchronizationContext.SetSynchronizationContext(new QtSynchronizationContext(this));
        }

        internal QGuiApplication(IntPtr existingApp)
            :base(CreateFromExisting(existingApp))
        {
            TriggerDelegate triggerDelegate = Trigger;
            _triggerHandle = GCHandle.Alloc(triggerDelegate);
            
            Interop.QGuiApplication.AddTriggerCallback(Handle, Marshal.GetFunctionPointerForDelegate(triggerDelegate));
            
            _oldSynchronizationContext = SynchronizationContext.Current;
            SynchronizationContext.SetSynchronizationContext(new QtSynchronizationContext(this));
        }

        public int Exec()
        {
            return Interop.QGuiApplication.Exec(Handle);
        }

        public void Dispatch(Action action)
        {
            lock (_actionQueue)
            {
                _actionQueue.Enqueue(action);
            }
            RequestTrigger();
        }

        public void Exit(int returnCode = 0)
        {
            Interop.QGuiApplication.Exit(Handle, returnCode);
        }

        public void Quit()
        {
            Exit();
        }

        private void RequestTrigger()
        {
            Interop.QGuiApplication.RequestTrigger(Handle);
        }

        internal IntPtr InternalPointer => Interop.QGuiApplication.InternalPointer(Handle);

        private void Trigger()
        {
            Action action;
            lock (_actionQueue)
            {
                action = _actionQueue.Dequeue();
            }
            action?.Invoke();
        }
        
        protected override void DisposeUnmanaged(IntPtr ptr)
        {
            SynchronizationContext.SetSynchronizationContext(_oldSynchronizationContext);
            Interop.QGuiApplication.Destroy(ptr);
            _triggerHandle.Free();
        }

        private static IntPtr CreateFromExisting(IntPtr app)
        {
            return Interop.QGuiApplication.Create(IntPtr.Zero, app);
        }

        private static IntPtr Create(List<string> args)
        {
            if (args == null)
            {
                args = new List<string>();
            }
            
            // By default, the argv[0] should be the process name.
            // .NET doesn't pass that name, but Qt should get it
            // since it does in a normal Qt environment.
            args.Insert(0, System.Diagnostics.Process.GetCurrentProcess().ProcessName);

            using (var strings = new NetVariantList())
            {
                foreach (var arg in args)
                {
                    using (var variant = new NetVariant())
                    {
                        variant.String = arg;
                        strings.Add(variant);
                    }
                }
                return Interop.QGuiApplication.Create(strings.Handle, IntPtr.Zero);
            }
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void TriggerDelegate();
        
        private class QtSynchronizationContext : SynchronizationContext
        {
            readonly QGuiApplication _guiApp;

            public QtSynchronizationContext(QGuiApplication guiApp)
            {
                _guiApp = guiApp;
            }

            public override void Post(SendOrPostCallback d, object state)
            {
                _guiApp.Dispatch(() => d.Invoke(state));
            }
        }
    }
    
    internal interface IQGuiApplicationInterop
    {
        [NativeSymbol(Entrypoint = "qguiapplication_create")]
        IntPtr Create(IntPtr args, IntPtr existingApp);
        [NativeSymbol(Entrypoint = "qguiapplication_destroy")]
        void Destroy(IntPtr app);

        [NativeSymbol(Entrypoint = "qguiapplication_exec")]
        int Exec(IntPtr app);
        [NativeSymbol(Entrypoint = "qguiapplication_addTriggerCallback")]
        void AddTriggerCallback(IntPtr app, IntPtr callback);
        [NativeSymbol(Entrypoint = "qguiapplication_requestTrigger")]
        void RequestTrigger(IntPtr app);
        [NativeSymbol(Entrypoint = "qguiapplication_exit")]
        void Exit(IntPtr app, int returnCode);
        [NativeSymbol(Entrypoint = "qguiapplication_internalPointer")]
        IntPtr InternalPointer(IntPtr app);
    }
}