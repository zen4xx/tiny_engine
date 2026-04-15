# Cascaded Shadow Mapping (CSM) - Документация

## Обзор

Cascaded Shadow Mapping - это техника рендеринга теней, которая разделяет область видимости камеры на несколько каскадов (зон) и рендерит карту теней отдельно для каждого каскада. Это позволяет достичь высокого качества теней как вблизи камеры, так и вдали.

## Архитектура реализации

### Структуры данных

#### `_CascadedShadowMap` (renderer_util.h:120-134)
```cpp
struct _CascadedShadowMap
{
    static const int CASCADE_COUNT = 4;
    VkImage depthImages[CASCADE_COUNT];           // Карты глубины для каждого каскада
    VmaAllocation depthImageMemories[CASCADE_COUNT];
    VkImageView depthImageViews[CASCADE_COUNT];   // Представления изображений
    VkFramebuffer framebuffers[CASCADE_COUNT];    // Фреймбуферы
    VkRenderPass renderPass;                      // Render pass для теней
    VkPipeline pipeline;                          // Pipeline для рендеринга теней
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;

    alignas(16) glm::mat4 lightViewProj[CASCADE_COUNT]; // Матрицы LightVP для каждого каскада
    float cascadeSplits[CASCADE_COUNT + 1];       // Границы каскадов
};
```

#### `_CascadedShadowMapData` (renderer_util.h:136-141)
Данные для передачи в шейдеры:
```cpp
struct _CascadedShadowMapData {
    alignas(16) glm::mat4 lightViewProj[4];  // Матрицы преобразования в пространство света
    alignas(16) glm::vec4 cascadeSplits;     // Дистанции переключения каскадов
    alignas(16) glm::vec3 lightDir;          // Направление света
    alignas(4)  int cascadeCount = 4;        // Количество каскадов
};
```

## Основные компоненты

### 1. Создание CSM (`createCascadedShadowMap`)

**Файл:** `renderer_util.cpp:1985`

Создает все необходимые ресурсы для CSM:
- **Depth images**: 4 текстуры глубины (2048x2048 каждая)
- **Render Pass**: Depth-only render pass
- **Pipeline**: Graphics pipeline с depth bias для предотвращения shadow acne
- **Framebuffers**: По одному на каждый каскад

**Ключевые параметры pipeline:**
```cpp
rasterizer.depthBiasEnable = VK_TRUE;
rasterizer.depthBiasConstantFactor = 1.25f;  // Смещение для борьбы с shadow acne
rasterizer.depthBiasSlopeFactor = 1.75f;     // Наклон-зависимое смещение
```

### 2. Обновление матриц каскадов (`updateCascadedShadowMatrices`)

**Файл:** `renderer_util.cpp:2168`

Вычисляет границы каскадов и матрицы преобразования:

#### Алгоритм вычисления границ каскадов:
```cpp
// Комбинация логарифмического и равномерного распределения
float logSplit = near * std::pow(far / near, f);
float uniformSplit = near + (far - near) * f;
cascadeSplits[i] = 0.9f * logSplit + 0.1f * uniformSplit;
```

Это дает больше разрешения вблизи камеры (где тени важнее) и меньше вдали.

#### Для каждого каскада:
1. Вычисляются 8 углов frustum в мировом пространстве
2. Находится центр и радиус ограничивающей сферы
3. Строится ортогональная камера света
4. Вычисляется матрица `lightOrtho * lightView`

### 3. Рендеринг карт теней (`recordShadowMapPass`)

**Файл:** `renderer_util.cpp:2242`

Для каждого из 4 каскадов:
1. Начинает render pass с соответствующим framebuffer
2. Биндит shadow pipeline
3. Рендерит все объекты с push constants (модель-матрица)
4. Завершает render pass

### 4. Теневой шейдер (`shadow.vert`)

**Файл:** `renderer/shaders/shadow.vert`

Простой vertex shader, который трансформирует вершины в пространство света:
```glsl
gl_Position = ubo_csm.lightViewProj[0] * pc.model * vec4(inPosition, 1.0);
```

### 5. Основной фрагментный шейдер с CSM (`default_shader.frag`)

**Файл:** `renderer/shaders/default_shader.frag`

#### Выбор каскада:
```glsl
float distFromCam = length(fragPos - fragCameraPos);
int cascadeIndex = 0;
if (distFromCam > csm.cascadeSplits.x) cascadeIndex = 1;
if (distFromCam > csm.cascadeSplits.y) cascadeIndex = 2;
if (distFromCam > csm.cascadeSplits.z) cascadeIndex = 3;
```

#### PCF (Percentage-Closer Filtering):
```glsl
float sampleShadowPCF(sampler2DShadow shadowMap, vec4 fragPosLightSpace)
{
    // Проекция координат в [0, 1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    // Проверка выхода за границы
    if (projCoords.z > 1.0 || ...) return 1.0;
    
    // PCF с ядром 5x5
    float shadow = 0.0;
    for (int x = -PCF_KERNEL_SIZE; x <= PCF_KERNEL_SIZE; ++x)
        for (int y = -PCF_KERNEL_SIZE; y <= PCF_KERNEL_SIZE; ++y)
            shadow += texture(shadowMap, vec3(projCoords.xy + offset, projCoords.z - SHADOW_BIAS));
    
    return shadow / 25.0; // Нормализация
}
```

## Descriptor Bindings

| Binding | Тип                         | Описание                    |
|---------|-----------------------------|-----------------------------|
| 0       | Uniform Buffer              | Основной UBO (view/proj)    |
| 1       | Combined Image Sampler      | Albedo текстура             |
| 2       | Combined Image Sampler      | Normal map                  |
| 3       | Combined Image Sampler      | Metal/Roughness             |
| 4       | Uniform Buffer              | CSM UBO                     |
| 5-8     | Combined Image Sampler      | Shadow maps (каскады 0-3)   |

## Настройка качества

### Разрешение карт теней
В `createCascadedShadowMap`:
```cpp
const uint32_t shadowMapRes = 2048; // Можно увеличить до 4096 для лучшего качества
```

### Количество каскадов
В `renderer_util.h`:
```cpp
static const int CASCADE_COUNT = 4; // Можно изменить (обычно 3-5)
```

### PCF параметры
В `default_shader.frag`:
```cpp
const float SHADOW_BIAS = 0.005f;     // Увеличить при shadow acne
const int PCF_KERNEL_SIZE = 2;        // 2 = ядро 5x5
const float PCF_STEP = 1.0 / 2048.0;  // Зависит от разрешения shadow map
```

### Распределение каскадов
В `updateCascadedShadowMatrices`:
```cpp
csm.cascadeSplits[i] = 0.9f * logSplit + 0.1f * uniformSplit;
// Измените соотношение 0.9/0.1 для настройки распределения
```

## Типичные проблемы и решения

### 1. Shadow Acne (полосы на освещенных поверхностях)
**Решение:** Увеличить `depthBiasConstantFactor` и `depthBiasSlopeFactor` в pipeline, или увеличить `SHADOW_BIAS` в шейдере.

### 2. Peter Panning (тени "отклеиваются" от объектов)
**Решение:** Уменьшить depth bias, но не слишком сильно (иначе появится shadow acne).

### 3. Aliasing на границах каскадов
**Решение:** 
- Увеличить разрешение shadow maps
- Добавить больше каскадов
- Использовать blended CSM (плавные переходы между каскадами)

### 4. Тени исчезают вдали
**Решение:** Увеличить `drawDistance` и настроить `cascadeSplits`.

## Интеграция в движок

### Инициализация
Вызывается автоматически при создании Renderer:
```cpp
_CascadedShadowMap m_csm; // В классе Renderer
```

### Обновление каждый кадр
Перед рендерингом сцены:
```cpp
updateCascadedShadowMatrices(
    m_csm,
    lightDir,      // Направление направленного света
    cameraPos,     // Позиция камеры
    cameraView,    // View матрица камеры
    cameraProj,    // Projection матрица камеры
    drawDistance   // Дальность прорисовки
);
```

### Рендеринг
1. Сначала рендерится shadow pass (`recordShadowMapPass`)
2. Затем основной pass с использованием shadow maps

## Производительность

### Оптимизации:
1. **Frustum culling**: Рендерить только объекты в текущем каскаде
2. **LOD для теней**: Меньше деталей для дальних каскадов
3. **Компрессия depth**: Использовать более компактные форматы глубины

### Балансировка:
- 4 каскада × 2048×2048 = ~64MB памяти
- Рендеринг сцены 4 раза для теней
- PCF 5×5 = 25 сэмплов на пиксель для теней

## Расширения

### Возможные улучшения:
1. **Blended CSM**: Плавные переходы между каскадами
2. **CSM + PSSM**: Per-pixel выбор каскада с интерполяцией
3. **Variance Shadow Maps**: Для мягких теней
4. **Temporal Reprojection**: Использование кадров из прошлого для улучшения качества

## Пример использования

```cpp
// Создание сцены
renderer.createScene("main");

// Добавление объектов
renderer.addObject("main", "object", vertices, indices, modelMatrix);

// Настройка освещения
renderer.setDirLight("main", glm::vec3(0.5f, -1.0f, 0.3f));
renderer.setDirLightColor("main", glm::vec3(1.0f, 1.0f, 0.9f));

// Обновление CSM перед рендерингом
updateCascadedShadowMatrices(m_csm, lightDir, cameraPos, view, proj, 100.0f);

// Рендеринг
renderer.drawScene("main");
```

## Ссылки

- [Original CSM Paper](https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-cascaded-shadow-maps)
- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [LearnOpenGL - Shadow Mapping](https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping)
