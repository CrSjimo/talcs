# Object-Binding

In TALCS, there is a type of operation where an object "manages" another object. This special type of management is called "object-binding".

Object-binding has the following features.

- **The managed object is opened or closed when the manager object is opened or closed (if it can be opened or closed).**
  
  For example, in a [MixerAudioSource](@ref talcs::MixerAudioSource) object, if an [AudioSource](@ref talcs::AudioSource) object is added to it, then the [AudioSource](@ref talcs::AudioSource) object will be opened or closed on the manager [MixerAudioSource](@ref talcs::MixerAudioSource) object opened or closed.

- **If the manager object has "some kind of" position property, the position property of the managed object will be changed on the position property of the manager object changed.**
  
  For example, in a [TransportAudioSource](@ref talcs::TransportAudioSource) object, on calling [TransportAudioSource::setPosition](@ref talcs::TransportAudioSource::setPotision())(), the [PositionableAudioSource::nextReadPosition](@ref talcs::PositionableAudioSource::nextReadPosition())() of the [PositionableAudioSource](@ref talcs::PositionableAudioSource) object managed by it will be also changed to the new position. Also, the position of the managed object is changed to the new position when it is bound to the manager object. Note that the position of the managed object is not "the same as" the manager object (though in many cases it does be), but is "set in accordance to the position of the manager object", such as [BufferingAudioSource](@ref talcs::BufferingAudioSource), where the position of the buffered [PositionableAudioSource](@ref talcs::PositionableAudioSource) is ahead.

- **If the manager object supports taking ownership, then the managed object will be deleted on the destruction of the manager object.**
  
  The existence of the `takeOwnership` parameter in relevant functions indicates that the manager object supports taking ownership of the managed objects. For example, in a [MixerAudioSource](@ref talcs::MixerAudioSource) object, if an [AudioSource](@ref talcs::AudioSource) object is added to it by calling [MixerAudioSource::addSource](@ref talcs::IMixer::addSource())() with the `takeOwnership` argument being true, then the [AudioSource](@ref talcs::AudioSource) object will be deleted when the [MixerAudioSource](@ref talcs::MixerAudioSource) object is deleted. The managed object is deleted using the `delete` keyword. Please take notice of the destruction order when utilizing this feature and only utilize this feature for convenience. If the destruction process is complicated or [QObject](https://doc.qt.io/qt-5/qobject.html) is involved, deleting these objects manually could be a better choice.

- **If the manager object is opened, taking the managed object away from it will not close nor delete the managed object.**
  
  The status of the managed object remains the same as the status before it is taken away.

- **Some manager objects support no managed objects.**
  
  For example, A [MixerAudioSource](@ref talcs::MixerAudioSource) object has no managed objects in the initial state. If the manager object only manages one object, the default value in relevant functions being `nullptr` indicates that the object supports no managed object.

Note that if an object is bound to a manager object, modifying it (e.g., changing the position) causes undefined behavior.
