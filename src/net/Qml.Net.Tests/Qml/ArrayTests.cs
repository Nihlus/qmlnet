﻿using System;
using FluentAssertions;
using Moq;
using Xunit;

namespace Qml.Net.Tests.Qml
{
    public class ArrayTests : BaseQmlTests<ArrayTests.ArrayTestsQml>
    {
        public class ArrayTestsQml
        {
            public virtual int[] GetArray()
            {
                return null;
            }

            public virtual void Test(object value)
            {
                
            }
        }

        [Fact]
        public void Can_get_length()
        {
            var array = new[] {3, 4, 6};
            Mock.Setup(x => x.GetArray()).Returns(array);
            Mock.Setup(x => x.Test(3));
            
            RunQmlTest(
                "test",
                @"
                    var array = Net.toJsArray(test.getArray())
                    test.test(array.length)
                ");
            
            Mock.Verify(x => x.GetArray(), Times.Once);
            Mock.Verify(x => x.Test(3), Times.Once);
        }
        
        [Fact]
        public void Can_get_indexed()
        {
            var array = new[] {3, 4, 6};
            Mock.Setup(x => x.GetArray()).Returns(array);
            Mock.Setup(x => x.Test(4));
            
            RunQmlTest(
                "test",
                @"
                    var array = Net.toJsArray(test.getArray())
                    test.test(array[1])
                ");
            
            Mock.Verify(x => x.GetArray(), Times.Once);
            Mock.Verify(x => x.Test(4), Times.Once);
        }
        
        [Fact]
        public void Can_set_indexed()
        {
            var array = new[] {3, 4, 6};
            Mock.Setup(x => x.GetArray()).Returns(array);
            
            RunQmlTest(
                "test",
                @"
                    var array = Net.toJsArray(test.getArray())
                    array[2] = 234
                ");
            
            Mock.Verify(x => x.GetArray(), Times.Once);
            array[2].Should().Be(234);
        }
        
        [Fact]
        public void Can_forEach()
        {
            var array = new[] {3, 4, 7};
            Mock.Setup(x => x.GetArray()).Returns(array);
            
            RunQmlTest(
                "test",
                @"
                    var array = Net.toJsArray(test.getArray())
                    array.forEach(function(value) {
                        test.test(value)
                    })
                ");
            
            Mock.Verify(x => x.GetArray(), Times.Once);
            Mock.Verify(x => x.Test(3), Times.Once);
            Mock.Verify(x => x.Test(4), Times.Once);
            Mock.Verify(x => x.Test(7), Times.Once);
        }

        [Fact]
        public void Can_not_push_for_array()
        {
            var array = new[] {3, 4, 7};
            Mock.Setup(x => x.GetArray()).Returns(array);
            Mock.Setup(x => x.Test(It.IsAny<object>()));
            
            RunQmlTest(
                "test",
                @"
                    var array = Net.toJsArray(test.getArray())
                    try {
                        array.push(23)
                    } catch(err) {
                        test.test(true)
                    }
                ");
            
            Mock.Verify(x => x.GetArray(), Times.Once);
            Mock.Verify(x => x.Test(true), Times.Once);
        }
    }
}